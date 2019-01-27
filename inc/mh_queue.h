#ifndef _MH_QUEUE_H_
#define _MH_QUEUE_H_

#include <stddef.h>
#include <stdint.h>

typedef volatile struct mh_queue {
    void *data_buf;
    size_t element_size;
    size_t latest_n;
} MH_QUEUE_T;

static inline void mh_queue_init(MH_QUEUE_T *q, void *data_buf, size_t element_size) {
    q->data_buf = data_buf;
    q->element_size = element_size;
    q->latest_n = 0;
}

static inline void mh_queue_enqueue(MH_QUEUE_T *q, const void *data) {
    asm volatile ("get_sem:\n\t"
                  "ldrex r4, %0\n\t"
                  "cmp r4, #0\n\t"
                  "beq sleep\n\t"
                  "ldr r6, %1\n\t"
                  "sub r4, #1\n\t"
                  "strex r5, r4, %0\n\t"
                  "cmp r5, #0\n\t"
                  "bne get_sem\n\t"
                  "dmb\n\t"
                  "b success\n\t"
                  "sleep:\n\t"
                  "wfi\n\t"
                  "b get_sem\n\t"
                  "success:"
                    : // no outputs
                    : "m" (q->latest_n), "m" (q->element_size), "r" (data)
                    : "r4", "r5"
    );
}

static inline void mh_queue_dequeue(MH_QUEUE_T *q, void *data) {
    asm volatile ("put_sem: ldrex r4, %0\n\t"
                    "mov r4, #1\n\t"
                    "strex r5, r4, %0\n\t"
                    "cmp r5, #0\n\t"
                    "bne put_sem\n\t"
                    "cmp %1, #1\n\t"
                    "dmb\n\t"
                    : //no outputs
                    : "m" (q->latest_n), "r" (q->latest_n), "m" (data)
                    : "r4", "r5"
    );
}

#endif /* _MH_QUEUE_H_ */
