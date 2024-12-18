#include "socket.h"
#include "log.h"
#include <limits.h>

namespace arvin {
static arvin::Logger::ptr g_logger = ARVIN_LOG_NAME("system");
Socket::ptr Socket::CreateTCP(arvin::Address::ptr address) {
  Socket::ptr sock(new Socket(address->getFamily(), TCP, 0));
  return sock;
}

Socket::ptr Socket::CreateUDP(arvin::Address::ptr address) {
  Socket::ptr sock(new Socket(address->getFamily(), UDP, 0));
  sock->newSock();
  sock->m_isConnected = true;
  return sock;
}

Socket::ptr Socket::CreateTCPSocket() {
  Socket::ptr sock(new Socket(IPv4, TCP, 0));
  return sock;
}

Socket::ptr Socket::CreateUDPSocket() {
  Socket::ptr sock(new Socket(IPv4, UDP, 0));
  sock->newSock();
  sock->m_isConnected = true;
  return sock;
}
Socket::ptr Socket::CreateTCPSocket6() {
  Socket::ptr sock(new Socket(IPv6, TCP, 0));
  return sock;
}

Socket::ptr Socket::CreateUDPSocket6() {
  Socket::ptr sock(new Socket(IPv6, UDP, 0));
  sock->newSock();
  sock->m_isConnected = true;
  return sock;
}

Socket::ptr Socket::CreateUnixTCPSocket() {
  Socket::ptr sock(new Socket(UNIX, TCP, 0));
  return sock;
}

Socket::ptr Socket::CreateUnixUDPSocket() {
  Socket::ptr sock(new Socket(UNIX, UDP, 0));
  return sock;
}
Socket::Socket(int family, int type, int protocol)
    : m_sock(-1), m_family(family), m_type(type), m_protocol(protocol),
      m_isConnected(false) {}

Socket::~Socket() { close(); }



} // namespace arvin