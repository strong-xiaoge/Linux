日志系统
===

作者:小哥

2020年6月25日

```mermaid
graph LR
A[Log功能框图]
	A --> B[可输出基本的日志信息]
	A --> C[可修改日志输出方式]
	A --> D[可以多种方式同时输出日志信息]
	A --> E[可设置日志输出的格式]
```



```mermaid
graph LR
A[Log分类]
	A --> B[日志类型]
		B --> B1[异常日志]
		B --> B2[业务日志]
		B --> B3[性能日志]
	A --> C[日志级别]
		C --> C1[错误日志]
		C --> C2[警告日志]
		C --> C3[通知日志]
		C --> C4[调试日志]
		C --> C5[跟踪日志]
	A --> D[日志格式]
		D --> D1[可设置]
	A --> E[日志输出方式]
		E --> E1[控制台]
		E --> E2[日志文件]
		E --> E3[其他设备]
```

## 已完成功能

#### 2020年7月4日

- 实现了日志的输出，默认输出到日志文件，可自定义输出方式
- 实现了日志的格式化，支持UNNKOWN、INFO、ERROR、WANRNING、DEBUG、TRAC格式，支持修改格式
- 重定向了<<，默认<<输出一次便格式化一次，<<支持字符（串）、数字
- 使用了双缓冲，缓冲区大小为4Kb，当前端缓冲区满时，将缓冲区交换并保存，若3s内未满，也没有更新缓冲区的信号也将自动更新，确保日志的及时性
- 使用了多线程，实现前后端，数据写入与输出分离
- 增加了原子操作和锁
- LOG的构造函数为（日志级别，日志名，自定义输出方式），所有参数均支持默认参数
- 重载了printf函数，此方式默认没有格式化
- 增加线程安全，增加了输出实例的修改接口
- 增加格式化实例函数，并增加了修改接口

#### 2020年7月8日

```C++
/**
 * @brief LOG构造函数
 * parameter : level : 日志级别,默认为UNKNOWN
 *             name : 日志输出文件名,默认为LOG.log
 *             output : 数据输出实现函数,默认输出到name文件中
 */
LOG(FORMAT::LOGLEVEL level,
    std::string name,
    bool (*output)(const OUTDATA*))

/**
 * @brief 更新后台数据
 */
void logUpdata(void)

/**
 * @brief 等待后台日志更新完成
 * parameter : outtime : 等待超时时间,默认为最大
 * return : 更新完成返回 true ，否则返回 false 
 */
bool LOG::weitlogUpdate(uint32_t outtime)

/**
 * @brief 设置日志输出文件名
 * parameter : name : 文件名
 */
void setLogNmae(std::string name)
  
/**
 * @brief 获取日志输出文件名
 * return : 文件名
 */
std::string getLogNmae(void)
    
/**
 * 重载了printf与<<
 * printf与stdio中的printf语法一样
 * <<仅支持字符（串），数字
 */
    
/**
 * LOG析构时会自动将未更新的数据更新完
 * LOG每隔3S也会更新一次数据
 */
    
/*其余的可执行查看源代码*/
```



