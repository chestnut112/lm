#include "../src/log.h"
#include <iostream>

int main(int argc, char **argv) {
  arvin::Logger::ptr logger(new arvin::Logger);
  logger->addAppender(arvin::LogAppender::ptr(new arvin::StdoutLogAppender));
  arvin::FileLogAppender::ptr file_appender(
      new arvin::FileLogAppender("./log.txt"));
  arvin::LogFormatter::ptr fmt(new arvin::LogFormatter("%d%T%p%T%m%n"));
  file_appender->setFormatter(fmt);
  file_appender->setLevel(arvin::LogLevel::ERROR);

  logger->addAppender(file_appender);

  // sylar::LogEvent::ptr event(new sylar::LogEvent(__FILE__, __LINE__, 0,
  // sylar::GetThreadId(), sylar::GetFiberId(), time(0))); event->getSS() <<
  // "hello sylar log"; logger->log(sylar::LogLevel::DEBUG, event);
  std::cout << "hello sylar log" << std::endl;

  ARVIN_LOG_INFO(logger) << "test macro";
  ARVIN_LOG_ERROR(logger) << "test macro error";

  ARVIN_LOG_FMT_ERROR(logger, "test macro fmt error %s", "aa");

  auto l = arvin::LoggerMgr::GetInstance()->getLogger("xx");
  ARVIN_LOG_INFO(l) << "xxx";
  return 0;
}
