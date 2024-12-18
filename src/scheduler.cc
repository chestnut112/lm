#include "scheduler.h"
#include "log.h"
// #include "macro.h"
#include "hook.h"

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
    /**
     * caller线程的主协程不会被线程的调度协程run进行调度，而且，线程的调度协程停止时，应该返回caller线程的主协程
     * 在user
     * caller情况下，把caller线程的主协程暂时保存起来，等调度协程结束时，再resume
     * caller协程
     */
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

void Scheduler::stop() {
  m_autoStop = true;
  if (m_rootFiber && m_threadCount == 0 &&
      (m_rootFiber->getState() == Fiber::TERM ||
       m_rootFiber->getState() == Fiber::INIT)) {
    ARVIN_LOG_INFO(g_logger) << this << " stopped";
    m_stopping = true;

    if (stopping()) {
      return;
    }
  }

  // bool exit_on_this_fiber = false;
  if (m_rootThread != -1) {
    // ARVIN_ASSERT(GetThis() == this);
  } else {
    // ARVIN_ASSERT(GetThis() != this);
  }

  m_stopping = true;
  for (size_t i = 0; i < m_threadCount; ++i) {
    tickle();
  }

  if (m_rootFiber) {
    tickle();
  }

  if (m_rootFiber) {
    // while(!stopping()) {
    //     if(m_rootFiber->getState() == Fiber::TERM
    //             || m_rootFiber->getState() == Fiber::EXCEPT) {
    //         m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0,
    //         true)); SYLAR_LOG_INFO(g_logger) << " root fiber is term, reset";
    //         t_fiber = m_rootFiber.get();
    //     }
    //     m_rootFiber->call();
    // }
    if (!stopping()) {
      m_rootFiber->call();
    }
  }

  std::vector<Thread::ptr> thrs;
  {
    MutexType::Lock lock(m_mutex);
    thrs.swap(m_threads);
  }

  for (auto &i : thrs) {
    i->join();
  }
  // if(exit_on_this_fiber) {
  // }
}
void Scheduler::setThis() { t_scheduler = this; }
void Scheduler::run() {
  ARVIN_LOG_DEBUG(g_logger) << m_name << " run";
  set_hook_enable(true);
  setThis();
  if (arvin::GetThreadId() != m_rootThread) {
    t_scheduler_fiber = Fiber::GetThis().get();
  }

  Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
  Fiber::ptr cb_fiber;

  FiberAndThread ft;
  while (true) {
    ft.reset();
    bool tickle_me = false;
    bool is_active = false;
    {
      MutexType::Lock lock(m_mutex);
      auto it = m_fibers.begin();
      while (it != m_fibers.end()) {
        if (it->thread != -1 && it->thread != arvin::GetThreadId()) {
          ++it;
          tickle_me = true;
          continue;
        }

        //ARVIN_ASSERT(it->fiber || it->cb);
        if (it->fiber && it->fiber->getState() == Fiber::EXEC) {
          ++it;
          continue;
        }

        ft = *it;
        m_fibers.erase(it++);
        ++m_activeThreadCount;
        is_active = true;
        break;
      }
      tickle_me |= it != m_fibers.end();
    }

    if (tickle_me) {
      tickle();
    }

    if (ft.fiber && (ft.fiber->getState() != Fiber::TERM &&
                     ft.fiber->getState() != Fiber::EXCEPT)) {
      ft.fiber->swapIn();
      --m_activeThreadCount;

      if (ft.fiber->getState() == Fiber::READY) {
        schedule(ft.fiber);
      } else if (ft.fiber->getState() != Fiber::TERM &&
                 ft.fiber->getState() != Fiber::EXCEPT) {
        ft.fiber->m_state = Fiber::HOLD;
      }
      ft.reset();
    } else if (ft.cb) {
      if (cb_fiber) {
        cb_fiber->reset(ft.cb);
      } else {
        cb_fiber.reset(new Fiber(ft.cb));
      }
      ft.reset();
      cb_fiber->swapIn();
      --m_activeThreadCount;
      if (cb_fiber->getState() == Fiber::READY) {
        schedule(cb_fiber);
        cb_fiber.reset();
      } else if (cb_fiber->getState() == Fiber::EXCEPT ||
                 cb_fiber->getState() == Fiber::TERM) {
        cb_fiber->reset(nullptr);
      } else { // if(cb_fiber->getState() != Fiber::TERM) {
        cb_fiber->m_state = Fiber::HOLD;
        cb_fiber.reset();
      }
    } else {
      if (is_active) {
        --m_activeThreadCount;
        continue;
      }
      if (idle_fiber->getState() == Fiber::TERM) {
        ARVIN_LOG_INFO(g_logger) << "idle fiber term";
        break;
      }

      ++m_idleThreadCount;
      idle_fiber->swapIn();
      --m_idleThreadCount;
      if (idle_fiber->getState() != Fiber::TERM &&
          idle_fiber->getState() != Fiber::EXCEPT) {
        idle_fiber->m_state = Fiber::HOLD;
      }
    }
  }
}

void Scheduler::tickle() { ARVIN_LOG_INFO(g_logger) << "tickle"; }

bool Scheduler::stopping() {
  MutexType::Lock lock(m_mutex);
  return m_autoStop && m_stopping && m_fibers.empty() &&
         m_activeThreadCount == 0;
}

void Scheduler::idle() {
  ARVIN_LOG_INFO(g_logger) << "idle";
  while (!stopping()) {
    arvin::Fiber::YieldToHold();
  }
}

void Scheduler::switchTo(int thread) {
  // ARVIN_ASSERT(Scheduler::GetThis() != nullptr);
  if (Scheduler::GetThis() == this) {
    if (thread == -1 || thread == arvin::GetThreadId()) {
      return;
    }
  }
  schedule(Fiber::GetThis(), thread);
  Fiber::YieldToHold();
}

std::ostream &Scheduler::dump(std::ostream &os) {
  os << "[Scheduler name=" << m_name << " size=" << m_threadCount
     << " active_count=" << m_activeThreadCount
     << " idle_count=" << m_idleThreadCount << " stopping=" << m_stopping
     << " ]" << std::endl
     << "    ";
  for (size_t i = 0; i < m_threadIds.size(); ++i) {
    if (i) {
      os << ", ";
    }
    os << m_threadIds[i];
  }
  return os;
}

SchedulerSwitcher::SchedulerSwitcher(Scheduler *target) {
  m_caller = Scheduler::GetThis();
  if (target) {
    target->switchTo();
  }
}

SchedulerSwitcher::~SchedulerSwitcher() {
  if (m_caller) {
    m_caller->switchTo();
  }
}

} // namespace arvin