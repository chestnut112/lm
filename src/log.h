#pragma once

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <vector>
#include <sstream>
#include <fstream>

namespace arvin
{ // 防止和别人的代码冲突

    class Logger; // <把Logger放到这里的目的?> 前面的一些类会用到Logger，不加会报未定义错误
    // 日志的一些配置
    class LogEvent
    {
    public:
        typedef std::shared_ptr<LogEvent> ptr; // [智能指针]
        LogEvent(const char *file, int32_t line, uint32_t eplase, uint32_t threadId, uint32_t fiberId, uint64_t time);

        const char *getFile() const { return m_file; }
        int32_t getLine() const { return m_line; }
        uint32_t getElapse() const { return m_elapse; }
        uint32_t getThreadId() const { return m_threadId; }
        uint32_t getFiberId() const { return m_threadId; }
        uint64_t getTime() const { return m_time; }
        const std::string getContent() const { return m_ss.str(); }
        std::stringstream &getSS() { return m_ss; }

    private:
        const char *m_file = nullptr; // 文件名
        int32_t m_line = 0;           // 行号，引入头文件stdint.h
        uint32_t m_elapse = 0;        // 程序启动到现在的毫秒
        uint32_t m_threadId = 0;      // 线程ID
        uint32_t m_fiberId = 0;       // 协程ID
        uint64_t m_time;              // 时间戳
        std::stringstream m_ss;       // 内容
    };

    class LogEventWrap
    {
    public:
        LogEventWrap(LogEvent::ptr e);
        ~LogEventWrap();
        LogEvent::ptr getEvent() const { return m_event; }

        std::stringstream &getSS();

    private:
        LogEvent::ptr m_event;
    }

    // 自定义日志级别
    class LogLevel
    {
    public:
        enum Level
        {
            UNKNOW = 0, //  未知 级别
            DEBUG = 1,  //  DEBUG 级别
            INFO = 2,   //  INFO 级别
            WARN = 3,   //  WARN 级别
            ERROR = 4,  //  ERROR 级别
            FATAL = 5   //  FATAL 级别
        };

        static const char *ToString(LogLevel::Level level);
    };

    // 日志格式
    class LogFormatter
    {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;
        LogFormatter(const std::string &pattern);
        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);

    public:
        class FormatItem
        { // [类中类]
        public:
            typedef std::shared_ptr<FormatItem> ptr;
            virtual ~FormatItem() {}
            virtual void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        };

        void init();

    private:
        std::string m_pattern;                // 解析格式
        std::vector<FormatItem::ptr> m_items; // 解析内容
    };

    // 日志输出地
    class LogAppender
    {
    public:
        typedef std::shared_ptr<LogAppender> ptr;
        virtual ~LogAppender() {} // 为了便于该类的派生类调用，定义为[虚类]，

        virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0; // [纯虚函数]，子类必须重写

        void setFormatter(LogFormatter::ptr val) { m_formatter = val; }
        LogFormatter::ptr getFormatter() const { return m_formatter; }

    protected:
        LogLevel::Level m_level = LogLevel::DEBUG; // 级别,为了便于子类访问该变量，设置在保护视图下
        LogFormatter::ptr m_formatter;             // 定义输出格式
    };

    // 日志输出器
    class Logger : public std::enable_shared_from_this<Logger>
    { // [?]
    public:
        typedef std::shared_ptr<Logger> ptr;

        Logger(const std::string &name = "root");

        void log(LogLevel::Level level, LogEvent::ptr event);

        // 不同级别的日志输出函数
        void debug(LogEvent::ptr event);
        void info(LogEvent::ptr event);
        void warn(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);
        void error(LogEvent::ptr event);

        void addAppender(LogAppender::ptr appender);          // 添加一个appender
        void delAppender(LogAppender::ptr appender);          // 删除一个appender
        LogLevel::Level getLevel() const { return m_level; }  // [const放在函数后]
        void setLevel(LogLevel::Level val) { m_level = val; } // 设置级别

        const std::string &getName() const { return m_name; }

    private:
        std::string m_name;                     // 日志名称
        LogLevel::Level m_level;                // 级别
        std::list<LogAppender::ptr> m_appender; // Appender集合,引入list
        LogFormatter::ptr m_formatter;
    };

    // 输出方法分类

    // 输出到控制台
    class StdoutAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<StdoutAppender> ptr;
        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;

    private:
    };

    // 输出到文件
    class FileLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
        FileLogAppender(const std::string &filename);                                      // 输出的文件名
        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override; // [override]

        bool reopen(); // 重新打开文件，成功返回true
    private:
        std::string m_filename;
        std::ofstream m_filestream; // stringstream要报错，引入sstream
    };
}
