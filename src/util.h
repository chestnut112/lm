#pragma once

#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdint.h>

namespace arvin
{
  // 获取线程ID
pid_t GetTreadId();

// 获取协程ID
uint32_t GetFiberId();
}