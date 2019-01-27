#ifndef _MH_TRANSMITTER_H_
#define _MH_TRANSMITTER_H_

#include <stdint.h>
#include <stddef.h>

void mh_transmitter_init(void);
void mh_transmitter_send(uint32_t byte);
void mh_transmitter_deinit(void);

#endif /* _MH_TRANSMITTER_H_ */
