/**
 * 作者:小哥
 * 时间:2020年7月8日
*/
#include "log.h"
#include<iostream>
#include<fstream>
#include<string.h>
#include <unistd.h>
#include<time.h>
#include<stdarg.h>

namespace log{
    /**
     * @brief 获取系统当前时间
    */
    char*getTime(void){
        time_t t = time(NULL);
        struct tm*time = localtime(&t);
        char*time_s=nullptr;
        if(time){
            time_s = asctime(time);
            int size=strlen(time_s);
            time_s[size-1]='\0';
            time->tm_year += 1900;
            time->tm_mon += 1;
        }
        return time_s;
    }
    /**
     * @brief 数据输出函数
    */
    bool outPut(const APPENDER::OUTDATA*out){
        FILE *fd = fopen(out->goalname.c_str(), "a");
        if(!fd)return false;
        if(fwrite(out->data, sizeof(char),out->len,fd)!=out->len)return false;
        if(fclose(fd)==EOF)return false;
        return true;
    }
    /**
     * @brief 数据格式化函数
    */
    std::string defForMat(FORMAT::LOGLEVEL level, std::string log){
        std::string str="[";
        switch(level){
            case FORMAT::INFO:str+="INFO";break;
            case FORMAT::ERROR:str+="ERROR";break;
            case FORMAT::WARNING:str+="WARN";break;
            case FORMAT::DEBUG:str+="DEBUG";break;
            case FORMAT::TRACE:str+="TRACE";break;
            default:str+="UNKNOWN";break;
        }
        str+="]";
        char*time=getTime();    //获取当前的系统时间
        str = str+" ["+time+"]";
        str = str+" : "+log+'\n';
        return str;
    }

    FORMAT::FORMAT(LOGLEVEL level=UNKNOWN){
        this->level=level;
        this->format=defForMat;
        fmtModflag=false;
    }
    /**
     * @brief 格式化日志信息
    */
    std::string FORMAT::forMat(LOGLEVEL level, std::string log){
        if(log.empty())return "";
        fmtModflag=true;
        std::string (*format)(FORMAT::LOGLEVEL, std::string)=this->format;
        fmtModflag=false;
        return format(level,log);
    }
    /**
     * @brief 设置格式化实现实例
    */
    bool FORMAT::setForMat(std::string (*format)(FORMAT::LOGLEVEL, std::string)){
        if(!format)return false;
        while(fmtModflag);//增加线程安全
        this->format=format;
        return true;
    }
    APPENDER::APPENDER(FORMAT::LOGLEVEL level=FORMAT::UNKNOWN,
                        bool (*output)(const OUTDATA*)=outPut,
                        std::string name="LOG.log"):FORMAT(level){
        filename=name;
        this->Output=output;
        outModflag=false;
        //开辟后端缓冲区
        outputbuffer = new char[buffersize];
    }
    APPENDER::~APPENDER()
    {
        if(this->bufferfullsize)append();
        if(outputbuffer){
            delete[] outputbuffer;
            outputbuffer=nullptr;
        }
    }
    /**
     * @brief 信息输出实现
    */
    void APPENDER::append(void){
        if(this->bufferfullsize){
             OUTDATA out={
                .data=this->outputbuffer,
                .len=this->bufferfullsize,
                .goalname=this->filename,
                .level=this->getLevel()
            };
            outModflag=true;
            bool (*output)(const OUTDATA*)= this->Output;
            outModflag=false;
            if(!output(&out))std::cerr<<"log out fail..."<<std::endl;
            this->bufferfullsize=0;
        }
    }
    /**
     * @brief 设置输出实现实例
    */
    bool APPENDER::setOutput(bool (*output)(const log::APPENDER::OUTDATA *)){
        if(!output)return false;
        while(outModflag);//增加线程安全
        this->Output=output;
        return true;
    }
    /**
    * @brief LOG构造函数
    * parameter : level : 日志级别,默认为UNKNOWN
    *             name : 日志输出文件名,默认为LOG.log
    *             output : 数据输出实现函数,默认输出到name文件中
    */
    LOG::LOG(FORMAT::LOGLEVEL level=FORMAT::UNKNOWN,
            std::string name="LOG.log",
            bool (*output)(const OUTDATA*)=outPut)
            :APPENDER(level,output,name){
        logbuffer = new char[buffersize];   //开辟前端缓冲区

        backstageexit=false;
        logupdataflag=false;
        bufferfullsize=0;
        //开辟缓冲区锁
        buffermutex = new std::mutex;
        //开辟后台记录线程
        backstagethread = new std::thread(&LOG::logOutput,this);   
    }
    LOG::~LOG(){
        if(this->backstagethread){
            //保存未更新的数据
            if(this->bufferfullsize)this->logUpdata();
            //发送后台结束指令
            backstageexit=true;
            //等待后台任务退出
            this->backstagethread->join();
            delete this->backstagethread;
            this->backstagethread=nullptr;
        }
        if(this->buffermutex){
            delete this->buffermutex;
            this->buffermutex=nullptr;
        }
        if(logbuffer){
            delete[] logbuffer;
            logbuffer=nullptr;
        }
    }
    /**
     * @brief 重载<<运算符
    */
    LOG& LOG::operator<<(const char*rvalue){
        if(rvalue){
            std::string log=this->FORMAT::forMat(FORMAT::getLevel(),rvalue);
            const uint32_t size = log.length();
            if(size > this->buffersize - this->bufferfullsize){
                /*前端缓冲区已满*/
                for(int i=0;i<100;++i){
                    if(weitlogUpdate(0xFFFF))break;
                }
            }
            buffermutex->lock();
            memcpy(this->logbuffer+this->bufferfullsize,log.c_str(),size);
            buffermutex->unlock();
            this->bufferfullsize += size;
        }
        return *this;
    }
    /**
     * @brief 重载printf输出
    */
    int LOG::printf(const char *fmt, ...){
        va_list args;
        va_start(args, fmt);
        int size=vsnprintf(&logbuffer[bufferfullsize],buffersize-bufferfullsize,fmt,args);
        if(size>0)bufferfullsize+=size;
        va_end(args);
        return size;
    }
    /**
     * @brief 更新后台信息
    */
    void LOG::logUpdata(){
        logupdataflag=true;
    }
    /**
     * @brief 等待后台日志更新完成
    */
    bool LOG::weitlogUpdate(uint32_t outtime=0XFFFF){
        this->logUpdata();
        while(outtime--){
            if(!logupdataflag)return true;
        }
        return false;
    }
    /**
     * @brief 更新日志信息
    */
    void LOG::logOutput(void){
        while(!backstageexit||logupdataflag){
            if(this->APPENDER::bufferfullsize){
                /*先将后台未及时记录的数据保存*/
                buffermutex->lock();
                this->APPENDER::append();
                buffermutex->unlock();
            }
            if(this->bufferfullsize){
                //交换前后缓冲区
                std::swap(this->APPENDER::outputbuffer,this->logbuffer);
                this->APPENDER::bufferfullsize=this->bufferfullsize;
                this->bufferfullsize=0;
                /*后台开始记录信息*/
                buffermutex->lock();
                this->APPENDER::append();
                buffermutex->unlock();
            }
            logupdataflag=false;
            for(int i=0;i<1000*3;++i){
                if(logupdataflag)break;
                if(!this->bufferfullsize&&backstageexit)return;
                usleep(1000);
            }
        }
    }
}
#ifdef TEST
    int main(){
        log::LOG info(log::FORMAT::INFO,"info.log");
        info<<"this is test code ...";
        info<<"the test code end ...";
        info.printf("test %d ...\n",1);
    }
#endif
