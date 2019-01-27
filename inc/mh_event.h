#ifndef _MH_EVENT_H_
#define _MH_EVENT_H_

typedef enum mh_event_type {
    MH_EVENT_READ_SENSOR_COMPLETE,
    MH_EVENT_READ_SENSOR_TIMEOUT,
    MH_EVENT_WAKEUP,
    MH_EVENT_MAX_EVENTS,
} MH_EVENT_TYPE_T;

typedef union mh_event_data {
    uint32_t temp_value;
} MH_EVENT_DATA_T;

typedef struct mh_event {
    MH_EVENT_TYPE_T type;
    MH_EVENT_DATA_T data;
} MH_EVENT_T;

void mh_state_machine(void);
void mh_event_put(const MH_EVENT_T *event);

#define MH_EVENT_PUT(event_type, event_data_field_name, event_data_value) do { \
    MH_EVENT_T event = {.type = event_type, .data.event_data_field_name = event_data_value}; \
    mh_event_put(&event); \
    } while (0) \

#endif /* _MH_EVENT_H_ */
