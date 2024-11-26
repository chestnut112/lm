#include "log.h"
#include <map>
#include <iostream>
#include <functional>
#include <time.h>
#include <string.h>

namespace arvin
{
  const char *LogLevel::ToString(LogLevel::Level level)
  {
    switch (level)
    {
#define XX(name)       \
  case LogLevel::name: \
    return #name;      \
    break;

      XX(DEBUG);
      XX(INFO);
      XX(WARN);
      XX(ERROR);
      XX(FATAL);
#undef XX
    default:
      return "UNKNOW";
    }
    return "UNKNOW";
  }

  LogLevel::Level LogLevel::FromString(const std::string &str)
  {
#define XX(level, v)        \
  if (str == #v)            \
  {                         \
    return LogLevel::level; \
  }
    XX(DEBUG, debug);
    XX(INFO, info);
    XX(WARN, warn);
    XX(ERROR, error);
    XX(FATAL, fatal);

    XX(DEBUG, DEBUG);
    XX(INFO, INFO);
    XX(WARN, WARN);
    XX(ERROR, ERROR);
    XX(FATAL, FATAL);
    return LogLevel::UNKNOW;
#undef XX
  }

  LogEventWrap::LogEventWrap(LogEvent::ptr e) : m_event(e)
  {
  }

  LogEventWrap::~LogEventWrap()
  {
    m_event->getLogger()->log(m_event->getLevel(), m_event);
  }

  void LogEvent::format(const char *fmt, ...)
  {
    va_list al;
    va_start(al, fmt);
    format(fmt, al);
    va_end(al);
  }

  void LogEvent::format(const char *fmt, va_list al)
  {
    char *buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if (len != -1)
    {
      m_ss << std::string(buf, len);
      free(buf);
    }
  }

  std::stringstream &LogEventWrap::getSS()
  {
    return m_event->getSS();
  }

  void LogAppender::setFormatter(LogFormatter::ptr val)
  {
    // MutexType::Lock lock(m_mutex);
    m_formatter = val;
    if (m_formatter)
    {
      m_hasFormatter = true;
    }
    else
    {
      m_hasFormatter = false;
    }
  }

  LogFormatter::ptr LogAppender::getFormatter()
  {
    // MutexType::Lock lock(m_mutex);
    return m_formatter;
  }

  class MessageFormatItem : public LogFormatter::FormatItem
  {
  public:
    MessageFormatItem(const std::string &str = "");
    void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
      os << event->getContent();
    }
  };

  class LevelFormatItem : public LogFormatter::FormatItem
  {
  public:
    LevelFormatItem(const std::string &str = "") {}
    void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
      os << LogLevel::ToString(level);
    }
  };

  class ElapseFormatItem : public LogFormatter::FormatItem
  {
  public:
    ElapseFormatItem(const std::string &str = "") {}
    void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
      os << event->getElapse();
    }
  };

  class NameFormatItem : public LogFormatter::FormatItem
  {
  public:
    NameFormatItem(const std::string &str = "") {}
    void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
      os << event->getLogger()->getName();
    }
  };

  class ThreadIdFormatItem : public LogFormatter::FormatItem
  {
  public:
    ThreadIdFormatItem(const std::string &str = "") {}
    void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
      os << event->getThreadId();
    }
  };

  class FiberIdFormatItem : public LogFormatter::FormatItem
  {
  public:
    FiberIdFormatItem(const std::string &str = "") {}
    void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
      os << event->getFiberId();
    }
  };

  class ThreadNameFormatItem : public LogFormatter::FormatItem
  {
  public:
    ThreadNameFormatItem(const std::string &str = "") {}
    void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
      os << event->getThreadName();
    }
  };

  class FilenameFormatItem : public LogFormatter::FormatItem
  {
  public:
    FilenameFormatItem(const std::string &str = "") {}
    void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
      os << event->getFile();
    }
  };

  class LineFormatItem : public LogFormatter::FormatItem
  {
  public:
    LineFormatItem(const std::string &str = "") {}
    void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
      os << event->getLine();
    }
  };

  class NewLineFormatItem : public LogFormatter::FormatItem
  {
  public:
    NewLineFormatItem(const std::string &str = "") {}
    void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
      os << std::endl;
    }
  };

  class StringFormatItem : public LogFormatter::FormatItem
  {
  public:
    StringFormatItem(const std::string &str)
        : m_string(str) {}
    void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
      os << m_string;
    }

  private:
    std::string m_string;
  };

  class TabFormatItem : public LogFormatter::FormatItem
  {
  public:
    TabFormatItem(const std::string &str = "") {}
    void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
      os << "\t";
    }

  private:
    std::string m_string;
  };

  class DateTimeFormatItem : public LogFormatter::FormatItem
  {
  public:
    DateTimeFormatItem(const std::string &format = "%Y-%m-%d %H:%M:%S")
        : m_format(format)
    {
      if (m_format.empty())
      {
        m_format = "%Y-%m-%d %H:%M:%S";
      }
    }

    void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
      struct tm tm;
      time_t time = event->getTime();
      localtime_r(&time, &tm);
      char buf[64];
      strftime(buf, sizeof(buf), m_format.c_str(), &tm);
      os << buf;
    }

  private:
    std::string m_format;
  };

  LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char *file, int32_t line, uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time, const std::string &thread_name)
      : m_file(file), m_line(line), m_elapse(elapse), m_threadId(thread_id), m_fiberId(fiber_id), m_time(time), m_threadName(thread_name), m_logger(logger), m_level(level)
  {
  }

  Logger::Logger(const std::string &name)
      : m_name(name), m_level(LogLevel::DEBUG)
  {
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
  }

  

}