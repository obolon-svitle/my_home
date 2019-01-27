#include <stddef.h>

#include "stm32f1xx.h"

#include "mh_transmitter.h"
#include "mh_gpio.h"
#include "mh_utils.h"
#include "mh_sleep.h"

#define TRANSMITTER_GPIO_PORT GPIOA
#define TRANSMITTER_GPIO_PIN 1

void mh_transmitter_init(void) {
    const MH_GPIO_CONFIG gpio_config = { .mode = MH_GPIO_MODE_2MHZ, .direction = MH_GPIO_DIRECTION_PP };

    RCC->APB2ENR |= (RCC_APB2ENR_IOPAEN);
    REG_DUMMY_READ(RCC->APB2ENR);

    mh_gpio_pin_config(TRANSMITTER_GPIO_PORT, TRANSMITTER_GPIO_PIN, &gpio_config);
}

void mh_transmitter_send(uint32_t byte) {
    register size_t bits = 8;
    MH_GPIO_PIN_STATE state;

    /* 
     * TODO: This is a very stupid noise-vulnureable protocol to transmit a byte.
     * Replace it with something more smart and energy efficient.
     */
#if 0
    asm volatile(
        "mov r4, #8\n\t"
        "ldr r5, %0\n\t"
        "mloop: subs r4, r4, #1\n\t"
        "mov r6, r5\n\t"
        "ands r6, #1\n\t"
        ""
        :
        : "m" (byte)
        : "r4", "r5", "r6"
    );
#endif

    while (bits--) {
        state = byte & 0x01 ? MH_GPIO_PIN_STATE_SET : MH_GPIO_PIN_STATE_RESET;
        mh_gpio_pin_write(TRANSMITTER_GPIO_PORT, TRANSMITTER_GPIO_PIN, state);
        busy_msleep(1);
        byte >>= 1;
    }
}

void mh_transmitter_deinit(void) {
    RCC->APB2ENR &= ~(RCC_APB2ENR_IOPAEN);
    REG_DUMMY_READ(RCC->APB2ENR);
}
