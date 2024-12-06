/**
 * @file macro.h
 * @brief 常用宏的封装
 * @author sylar.yin
 * @email 564628276@qq.com
 * @date 2019-06-01
 * @copyright Copyright (c) 2019年 sylar.yin All rights reserved (www.sylar.top)
 */
#ifndef __SYLAR_MACRO_H__
#define __SYLAR_MACRO_H__

#include <string.h>
#include <assert.h>
#include "log.h"
#include "util.h"

#if defined __GNUC__ || defined __llvm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立
#   define ARVIN_LIKELY(x)       __builtin_expect(!!(x), 1)
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#   define ARVIN_UNLIKELY(x)     __builtin_expect(!!(x), 0)
#else
#   define ARVIN_LIKELY(x)      (x)
#   define ARVIN_UNLIKELY(x)      (x)
#endif

/// 断言宏封装
#define ARVIN_ASSERT(x) \
    if(ARVIN_UNLIKELY(!(x))) { \
        ARVIN_LOG_ERROR(ARVIN_LOG_ROOT()) << "ASSERTION: " #x \
            << "\nbacktrace:\n" \
            << arvin::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

/// 断言宏封装
#define ARVIN_ASSERT2(x, w) \
    if(ARVIN_UNLIKELY(!(x))) { \
       ARVIN_LOG_ERROR(ARVIN_LOG_ROOT()) << "ASSERTION: " #x \
            << "\n" << w \
            << "\nbacktrace:\n" \
            << arvin::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

#endif