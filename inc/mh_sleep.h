#ifndef _MH_SLEEP_H_
#define _MH_SLEEP_H_

#include "stm32f1xx.h"

//how much cpu cycles busy_loop() performs
#define CYCLE_COUNT 3

#define LOOPS_PER_SECOND (SystemCoreClock / CYCLE_COUNT)

static inline void busy_loop(uint32_t loop_num) {
    asm volatile ("ldr r4, %0\n\t"  // 2 cycles
                    "loop: subs r4, r4, #1\n\t"     // 1
                    "bne loop\n\t" // 1
                    : /* No outputs */
                    : "m" (loop_num)
                    : "r4"
                    );
}

static inline void busy_sleep(uint32_t seconds) {
    busy_loop(LOOPS_PER_SECOND * seconds);
}

static inline void busy_msleep(uint32_t mseconds) {
    busy_loop(LOOPS_PER_SECOND / 1000 * mseconds);
}

#endif /* _MH_SLEEP_H_ */
