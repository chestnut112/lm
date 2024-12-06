#include "fiber.h"
#include "config.h"
#include "log.h"
// #include "macro.h"
#include <atomic>
// #include "scheduler.h"

namespace arvin {
static Logger::ptr g_logger = ARVIN_LOG_NAME("system");
/// 全局静态变量，用于生成协程id
static std::atomic<uint64_t> s_fiber_id{0};
/// 全局静态变量，用于统计当前的协程数
static std::atomic<uint64_t> s_fiber_count{0};
/// 线程局部变量，当前线程正在运行的协程
static thread_local Fiber *t_fiber = nullptr;
/// 线程局部变量，当前线程的主协程，切换到这个协程，就相当于切换到了主线程中运行，智能指针形式
static thread_local Fiber::ptr t_thread_Fiber = nullptr;

// 协程栈大小，可通过配置文件获取，默认128k
static ConfigVar<uint32_t>::ptr g_fiber_stack_size = Config::Lookup<uint32_t>(
    "fiber.stack_size", 128 * 1024, "fiber stack size");

/**
 * @brief malloc栈内存分配器
 */
class MallocStackAllocator {
public:
  static void *Alloc(size_t size) { return malloc(size); }
  static void Dealloc(void *vp, size_t size) { return free(vp); }
};

using StackAllocator = MallocStackAllocator;

uint64_t Fiber::GetFiberId() {
  if (t_fiber) {
    return t_fiber->getId();
  }
  return 0;
}

Fiber::Fiber() {
  m_state = EXEC;
  SetThis(this);
  if (getcontext(&m_ctx)) {
    // ARVIN_ASSERT2(false, "getcontext");
  }
  ++s_fiber_count;
  ARVIN_LOG_DEBUG(g_logger) << "Fiber::Fiber main";
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool use_caller)
    : m_id(++s_fiber_id), m_cb(cb) {
  ++s_fiber_count;
  m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();

  m_stack = StackAllocator::Alloc(m_stacksize);
  if (getcontext(&m_ctx)) {
    // ARVIN_ASSERT2(false, "getcontext");
  }
  m_ctx.uc_link = nullptr;
  m_ctx.uc_stack.ss_sp = m_stack;
  m_ctx.uc_stack.ss_size = m_stacksize;

  if (!use_caller) {
    makecontext(&m_ctx, &Fiber::MainFunc, 0);
  } else {
    makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
  }

  ARVIN_LOG_DEBUG(g_logger) << "Fiber::Fiber id=" << m_id;
}

Fiber::~Fiber() {
  --s_fiber_count;
  if (m_stack) {
    // 有栈，说明是子协程，需要确保子协程一定是结束状态
    // ARVIN_ASSERT(m_state == TERM || m_state == EXCEPT || m_state == INIT);
    StackAllocator::Dealloc(m_stack, m_stacksize);
  } else {
    // 没有栈，说明是线程的主协程
    // ARVIN_ASSERT(!m_cb);
    // ARVIN_ASSERT(m_state == EXEC);

    Fiber *cur = t_fiber;
    if (cur == this) {
      SetThis(nullptr);
    }
  }
  ARVIN_LOG_DEBUG(g_logger)
      << "Fiber::~Fiber id=" << m_id << " total=" << s_fiber_count;
}

// 重置协程函数，并重置状态
// INIT，TERM, EXCEPT
void Fiber::reset(std::function<void()> cb) {
  //ARVIN_ASSERT(m_stack);
  //ARVIN_ASSERT(m_state == TERM || m_state == EXCEPT || m_state == INIT);
  m_cb = cb;
  if (getcontext(&m_ctx)) {
    //ARVIN_ASSERT2(false, "getcontext");
  }

  m_ctx.uc_link = nullptr;
  m_ctx.uc_stack.ss_sp = m_stack;
  m_ctx.uc_stack.ss_size = m_stacksize;

  makecontext(&m_ctx, &Fiber::MainFunc, 0);
  m_state = INIT;
}

void Fiber::call() {
    SetThis(this);
    m_state = EXEC;
    if(swapcontext(&t_thread_Fiber->m_ctx, &m_ctx)) {
        //ARVIN_ASSERT2(false, "swapcontext");
    }
}



void Fiber::SetThis(Fiber *f) { t_fiber = f; }

} // namespace arvin