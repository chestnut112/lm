#include "scheduler.h"
#include "log.h"
// #include "macro.h"
//  #include "hook.h"

namespace arvin {

static arvin::Logger::ptr g_logger = ARVIN_LOG_NAME("system");

/// 当前线程的调度器，同一个调度器下的所有线程共享同一个实例
static thread_local Scheduler *t_scheduler = nullptr;
/// 当前线程的调度协程，每个线程都独有一份
static thread_local Fiber *t_scheduler_fiber = nullptr;

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string &name)
    : m_name(name) {
  // ARVIN_ASSERT(threads>0);
  if (use_caller) {
    arvin::Fiber::GetThis();
    --threads;
    // ARVIN_ASSERT(GetThis() == nullptr);
    t_scheduler = this;
    m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
    arvin::Thread::SetName(m_name);
    t_scheduler_fiber = m_rootFiber.get();
    m_rootThread = arvin::GetThreadId();
    m_threadIds.push_back(m_rootThread);
  } else {
    m_rootThread = -1;
  }
  m_threadCount = threads;
}

Scheduler::~Scheduler() {
  // ARVIN_ASSERT(m_stopping);
  if (GetThis() == this) {
    t_scheduler = nullptr;
  }
}

Fiber *Scheduler::GetMainFiber() { return t_scheduler_fiber; }

void Scheduler::start() {
  MutexType::Lock lock(m_mutex);
  if (!m_stopping) {
    return;
  }
  m_stopping = false;
  // ARVIN_ASSERT(m_threads.empty());

  m_threads.resize(m_threadCount);
  for (size_t i = 0; i < m_threadCount; ++i) {
    m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this),
                                  m_name + "_" + std::to_string(i)));
    m_threadIds.push_back(m_threads[i]->getId());
  }
  lock.unlock();
}

void Scheduler::stop() {}
void Scheduler::setThis() { t_scheduler = this; }

} // namespace arvin