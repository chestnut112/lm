#include "timer.h"
#include "util.h"

namespace arvin {
bool Timer::Comparator::operator()(const Timer::ptr &lhs,
                                   const Timer::ptr &rhs) const {
  if (!lhs && !rhs) {
    return false;
  }
  if (!lhs) {
    return true;
  }
  if (!rhs) {
    return false;
  }
  if (lhs->m_next < rhs->m_next) {
    return true;
  }
  if (rhs->m_next < lhs->m_next) {
    return false;
  }
  return lhs.get() < rhs.get();
}



} // namespace arvin