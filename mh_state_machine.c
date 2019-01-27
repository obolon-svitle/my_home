#include <stdint.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "mh_state_machine.h"
#include "mh_timer.h"
#include "mh_event.h"
#include "mh_semaphore.h"
#include "mh_queue.h"
#include "mh_transmitter.h"

#include "mh_sensor.h"
#include "mh_log.h"
#include "mh_utils.h"

#ifdef MH_DEBUG
#define IDLE_TIMEOUT_SEC (5)
#else
#define IDLE_TIMEOUT_SEC (360)
#endif

typedef enum mh_state {
    MH_STATE_INIT,
    MH_STATE_READ_SENSOR_DATA,
    MH_STATE_IDLE,
    MH_STATE_DEINIT,
    MH_STATE_MAX_STATES,
} MH_STATE_T;

#ifdef MH_DEBUG
static const char* const mh_state_str[] = {
    E_STR(MH_STATE_INIT),
    E_STR(MH_STATE_READ_SENSOR_DATA),
    E_STR(MH_STATE_IDLE),
    E_STR(MH_STATE_DEINIT),
};

static const char* const mh_event_str[] = {
    E_STR(MH_EVENT_READ_SENSOR_COMPLETE),
    E_STR(MH_EVENT_READ_SENSOR_TIMEOUT),
    E_STR(MH_EVENT_WAKEUP),
};
#endif

typedef enum state_action {
    STATE_EXIT,
    STATE_ENTER,
} STATE_ACTION_T;

#define EVENT_MAX_NUM 5

static MH_EVENT_T _pending_events[EVENT_MAX_NUM];
static MH_QUEUE_T _q;

#ifdef MH_DEBUG
static inline const char* mh_state_to_str(MH_STATE_T state) {
    return mh_state_str[state];
}
#endif

static inline void state_change(MH_STATE_T state, STATE_ACTION_T action) {
    switch (state) {
        case MH_STATE_READ_SENSOR_DATA:
            switch (action) {
                case STATE_ENTER:
                    mh_sensor_read_start();
                    break;
                case STATE_EXIT:
                    mh_sensor_read_stop();
                    break;
                default:
                    MH_ASSERT(0);
                    break;
            }
            break;
        case MH_STATE_IDLE:
            switch (action) {
                case STATE_ENTER:
                    mh_timer_start(MH_TIMER_IDLE, IDLE_TIMEOUT_SEC);
                case STATE_EXIT:
                    break;
                default:
                    MH_ASSERT(0);
                    break;
            }
            break;
        default:
            break;
    }
}

static inline void state_transition(MH_STATE_T * const state, MH_STATE_T new_state) {
    MH_LOGD("Leaving state [%s]", mh_state_to_str(*state));
    state_change(*state, STATE_EXIT);
    MH_LOGD("Entering state [%s]", mh_state_to_str(new_state));
    *state = new_state;
    state_change(new_state, STATE_ENTER);
}

static inline void mh_event_get(MH_EVENT_T *event) {
    mh_queue_dequeue(&_q, event);
    MH_LOGD("Got event [%s]", mh_event_to_str(event));
}

void mh_event_put(const MH_EVENT_T *event) {
    MH_LOGD_ISR("Put event [%s]", mh_event_to_str(event));
    mh_queue_enqueue(&_q, event);
}

void mh_state_machine_init(void) {
    mh_queue_init(&_q, _pending_events, ARRAY_SIZE(_pending_events));
}

void mh_state_machine_deinit(void) {
}

void mh_state_machine_run(void) {
    MH_STATE_T state = MH_STATE_INIT;
    MH_EVENT_T event;

    MH_ASSERT(MH_STATE_MAX_STATES == ARRAY_SIZE(mh_state_str));
    MH_ASSERT(MH_EVENT_MAX_EVENTS == ARRAY_SIZE(mh_event_str));

    state_transition(&state, MH_STATE_READ_SENSOR_DATA);

    while (state != MH_STATE_DEINIT) {
        mh_event_get(&event);

        switch (event.type) {
            case MH_EVENT_READ_SENSOR_COMPLETE:
                switch (state) {
                    case MH_STATE_READ_SENSOR_DATA:
                        mh_transmitter_send(event.data.temp_value);
                        state_transition(&state, MH_STATE_IDLE);
                        break;
                    default:
                        break;
                }
                break;
            case MH_EVENT_READ_SENSOR_TIMEOUT:
                switch (state) {
                    case MH_STATE_READ_SENSOR_DATA:
                        state_transition(&state, MH_STATE_IDLE);
                        break;
                    default:
                        break;
                }
                break;
            case MH_EVENT_WAKEUP:
                switch (state) {
                    case MH_STATE_IDLE:
                        state_transition(&state, MH_STATE_READ_SENSOR_DATA);
                        break;
                    default:
                        break;
                }
                break;
            default:
                MH_ASSERT(0);
                break;
        }
    }
}
