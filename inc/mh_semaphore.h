#ifndef _MH_SEMAPHORE_H_
#define _MH_SEMAPHORE_H_

typedef volatile struct mh_semaphore {
    int counter;
} MH_SEMAPHORE_T;

static inline void mh_bsemaphore_get(MH_SEMAPHORE_T *semphr) {
    asm volatile (  "get_sem: ldrex r4, %0\n\t"
                    "cmp r4, #0\n\t"
                    "beq sleep\n\t"
                    "sub r4, #1\n\t"
                    "strex r5, r4, %0\n\t"
                    "cmp r5, #0\n\t"
                    "bne get_sem\n\t"
                    "dmb\n\t"
                    "b success\n\t"
                    "sleep: wfi\n\t"
                    "b get_sem\n\t"
                    "success:"
                    : // no outputs
                    : "m" (semphr->counter)
                    : "r4", "r5"
        );
}

static inline void mh_bsemaphore_put(MH_SEMAPHORE_T *semphr) {
    asm volatile (  "put_sem: ldrex r4, %0\n\t"
                    "mov r4, #1\n\t"
                    "strex r5, r4, %0\n\t"
                    "cmp r5, #0\n\t"
                    "bne put_sem\n\t"
                    "cmp %1, #1\n\t"
                    "dmb\n\t"
                    : //no outputs
                    : "m" (semphr->counter), "r" (semphr->counter)
                    : "r4", "r5"
        );
}

#endif /* _SEMAPHORE_H_ */
