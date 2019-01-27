#ifndef _MH_UTILS_H_
#define _MH_UTILS_H_

#include <stdint.h>
#include <stdio.h>

#include "mh_log.h"

#define APB1_CLOCK (SystemCoreClock / APBPrescTable[(RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos])

#define REG_DUMMY_READ(reg) do { \
    volatile uint32_t reg_ptr = (reg); \
    (void) reg_ptr; \
} while (0) \

#ifdef MH_DEBUG
#define MH_ASSERT(expr) if (!(expr)) { MH_LOGE("Assertion error: " #expr); for (;;);}
#else
#define MH_ASSERT(expr)
#endif

#ifndef __STRING
#define __STRING(s) #s
#endif

#define E_STR(x) [x] = __STRING(x)

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

#endif /* _MH_UTILS_H_ */
