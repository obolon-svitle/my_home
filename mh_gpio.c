#include "stm32f1xx.h"

#include "mh_gpio.h"
#include "mh_log.h"
#include "mh_utils.h"

#define GPIO_PIN_MAX 16

void mh_gpio_pin_config(GPIO_TypeDef *GPIOx, uint32_t pin_number, const MH_GPIO_CONFIG *gpio_config) {
    uint32_t config = 0x00U;
    __IO uint32_t *configregister;
    uint32_t registeroffset = 0U;

    MH_ASSERT(pin_number < GPIO_PIN_MAX);
    MH_ASSERT(GPIOx != NULL);
    MH_ASSERT(gpio_config != NULL);

    config = gpio_config->direction + gpio_config->mode; /* High freq + mode */

    /* Check if the current bit belongs to first half or last half of the pin_number count number
    in order to address CRH or CRL register*/
    configregister = (pin_number < 8) ? &GPIOx->CRL : &GPIOx->CRH;
    registeroffset = (pin_number < 8) ? (pin_number << 2U) : ((pin_number - 8) << 2 );

    MODIFY_REG((*configregister), ((GPIO_CRL_MODE0 | GPIO_CRL_CNF0) << registeroffset), (config << registeroffset));
}
