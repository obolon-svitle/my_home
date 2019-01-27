#ifndef _MH_TIMER_H_
#define _MH_TIMER_H_

#include <stdint.h>
#include <stddef.h>

typedef enum mh_timer {
    //Put any new timer definition here
    MH_TIMER_IDLE,
    MH_TIMER_MAX,
} MH_TIMER_T;

void mh_timer_init(void);

void mh_timer_start(MH_TIMER_T timer_id, uint32_t timeout);

void mh_timer_deinit(void);

#endif /* _MH_TIMER_H_ */
