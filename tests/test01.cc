#include <iostream>
#include "../src/log.h"

int main(int argc, char** argv) {
    arvin::Logger::ptr logger(new arvin::Logger);
    logger->addAppender(arvin::LogAppender::ptr(new arvin::StdoutAppender)); 

    arvin::LogEvent::ptr event(new arvin::LogEvent(__FILE__, __LINE__, 1, 2, 3, time(0)));
    logger->log(arvin::LogLevel::DEBUG, event);

    return 0;
}

