#include "iomanager.h"
#include "log.h"
#include "macro.h"
#include <fcntl.h>     // for fcntl()
#include <sys/epoll.h> // for epoll_xxx()
#include <unistd.h>    // for pipe()

namespace arvin {
static arvin::Logger::ptr g_logger = ARVIN_LOG_NAME("system");
enum EpollCtlOp {};
static std::ostream &operator<<(std::ostream &os, const EpollCtlOp &op) {
  switch ((int)op) {
#define XX(ctl)                                                                \
  case ctl:                                                                    \
    return os << #ctl;
    XX(EPOLL_CTL_ADD);
    XX(EPOLL_CTL_MOD);
    XX(EPOLL_CTL_DEL);
#undef XX
  default:
    return os << (int)op;
  }
}

static std::ostream &operator<<(std::ostream &os, EPOLL_EVENTS events) {
  if (!events) {
    return os << "0";
  }
  bool first = true;
#define XX(E)                                                                  \
  if (events & E) {                                                            \
    if (!first) {                                                              \
      os << "|";                                                               \
    }                                                                          \
    os << #E;                                                                  \
    first = false;                                                             \
  }
  XX(EPOLLIN);
  XX(EPOLLPRI);
  XX(EPOLLOUT);
  XX(EPOLLRDNORM);
  XX(EPOLLRDBAND);
  XX(EPOLLWRNORM);
  XX(EPOLLWRBAND);
  XX(EPOLLMSG);
  XX(EPOLLERR);
  XX(EPOLLHUP);
  XX(EPOLLRDHUP);
  XX(EPOLLONESHOT);
  XX(EPOLLET);
#undef XX
  return os;
}

IOManager::FdContext::EventContext &
IOManager::FdContext::getContext(IOManager::Event event) {
  switch (event) {
  case IOManager::READ:
    return read;
  case IOManager::WRITE:
    return write;
  default:
    //ARVIN_ASSERT2(false, "getContext");
  }
  throw std::invalid_argument("getContext invalid event");
}



} // namespace arvin