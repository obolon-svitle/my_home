#ifndef _MH_GPIO_H_
#define _MH_GPIO_H_

#include <stdint.h>

#include "stm32f1xx.h"

typedef enum mh_gpio_mode {
    MH_GPIO_MODE_10MHZ = 0x01,
    MH_GPIO_MODE_2MHZ  = 0x02,
    MH_GPIO_MODE_50MHZ = 0x03,
} MH_GPIO_MODE;

typedef enum mh_gpio_direction {
    MH_GPIO_DIRECTION_PP    = (0x0 << 2),
    MH_GPIO_DIRECTION_OD    = (0x1 << 2),
    MH_GPIO_DIRECTION_AF_PP = (0x2 << 2),
    MH_GPIO_DIRECTION_AF_OD = (0x3 << 2),
} MH_GPIO_DIRECTION;

typedef enum mh_gpio_pin_state {
    MH_GPIO_PIN_STATE_RESET,
    MH_GPIO_PIN_STATE_SET,
} MH_GPIO_PIN_STATE;

typedef struct mh_gpio_config {
    MH_GPIO_MODE mode;
    MH_GPIO_DIRECTION direction;
} MH_GPIO_CONFIG;

static inline void mh_gpio_pin_write(GPIO_TypeDef *GPIOx, uint32_t pin, MH_GPIO_PIN_STATE state) {
    GPIOx->BSRR = ((1 << pin) << (state == MH_GPIO_PIN_STATE_RESET ? 16 : 0));
}

void mh_gpio_pin_config(GPIO_TypeDef *GPIOx, uint32_t pin, const MH_GPIO_CONFIG *gpio_config);

#endif /* _MH_GPIO_H_ */
