/**
 * 作者:小哥
 * 时间:2020年7月8日
*/
#pragma one
#include<string>
#include<thread>
#include<mutex>
#include <atomic>

namespace log{
    /*缓冲区大小*/
    #define BUFFSIZE 1024*4 //16kb
    /**
     * @brief 日志格式
     * */
    class FORMAT
    {
    public:
    /*日志级别枚举*/
        enum LOGLEVEL{
            UNKNOWN=0,
            INFO=1,
            ERROR=2,
            WARNING=3,
            DEBUG=4,
            TRACE=5,
        };
        FORMAT(LOGLEVEL);
        void setLevel(LOGLEVEL level){
            if(level<UNKNOWN && level>TRACE)return;
            this->level=level;
        }
        LOGLEVEL getLevel(void){
            return this->level;
        }
        std::string forMat(LOGLEVEL,std::string);//格式化日志消息
        bool setForMat(std::string (*format)(FORMAT::LOGLEVEL, std::string));
    private:
        std::atomic<bool> fmtModflag;
        std::string (*format)(FORMAT::LOGLEVEL, std::string); 
        LOGLEVEL level=UNKNOWN;
    };
    /** 
     * @brief 日志输出
    */
    class APPENDER:public FORMAT
    {
    public:
        /*输出数据*/
        typedef struct{
            const char*data;
            const uint32_t len;
            const std::string goalname;
            const FORMAT::LOGLEVEL level;
        }OUTDATA;
        APPENDER(FORMAT::LOGLEVEL level,
                bool (*output)(const OUTDATA*),
                std::string name);
        ~APPENDER();
        void setLogNmae(std::string name) { 
            filename=name;
        }
        std::string getLogNmae(void){
            return filename;
        }
        void append(void);   //信息输出
        bool setOutput(bool(*output)(const OUTDATA*));
    private:
        const uint32_t buffersize=BUFFSIZE;
        std::atomic<bool> outModflag;
        std::string filename;//日志输出文件名
        bool (*Output)(const OUTDATA*);
    protected:
        char*outputbuffer=nullptr;  //输出缓冲区
        uint32_t bufferfullsize=0;  //缓冲区记录字节数
    };
    /**
     * @brief 日志器
    */
    class LOG:public APPENDER
    {
    public:
        LOG(FORMAT::LOGLEVEL level,
            std::string name,
            bool (*output)(const OUTDATA*));
        ~LOG();
        //更新后台数据
        void logUpdata(void);
        //等待后台数据更新完成
        bool weitlogUpdate(uint32_t outtime);
        //重载printf
        int printf(const char*fmt,...);
        //重载<<运算符
        LOG& operator <<(const char*);    
        LOG& operator <<(const char rvalue){
            return *this<<&rvalue;
        }
        LOG& operator <<(const short rvalue){
            return *this<<std::to_string(rvalue).c_str();
        }
        LOG& operator <<(const int rvalue){
            return *this<<std::to_string(rvalue).c_str();
        }
        LOG& operator <<(const long rvalue){
            return *this<<std::to_string(rvalue).c_str();
        }
        LOG& operator <<(const float rvalue){
            return *this<<std::to_string(rvalue).c_str();
        }
        LOG& operator <<(const double rvalue){
            return *this<<std::to_string(rvalue).c_str();
        }
        LOG& operator <<(const unsigned char rvalue){
            return *this<<std::to_string(rvalue).c_str();
        }
        LOG& operator <<(const unsigned short rvalue){
            return *this<<std::to_string(rvalue).c_str();
        }
        LOG& operator <<(const unsigned int rvalue){
            return *this<<std::to_string(rvalue).c_str();
        }
        LOG& operator <<(const unsigned long rvalue){
            return *this<<std::to_string(rvalue).c_str();
        }
    private:
        const uint32_t buffersize=BUFFSIZE;
        std::atomic<uint32_t> bufferfullsize;  //缓冲区记录字节数
        char*logbuffer=nullptr;     //日志缓冲区
        std::mutex*buffermutex=nullptr;     //缓冲区锁

        void logOutput(void);   //后台信息更新
        std::thread*backstagethread=nullptr; //后台线程
        std::atomic<bool> backstageexit;   //后台记录任务退出标志
        std::atomic<bool> logupdataflag; //数据更新标志位(原子锁)
    };
}
