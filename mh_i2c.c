#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "stm32f1xx.h"
#include "stm32f1xx_hal_i2c.h"

#include "mh_i2c.h"
#include "mh_log.h"
#include "mh_gpio.h"

#include "mh_utils.h"

#define ENABLE_EVT_INTERRUPT
//#define ENABLE_BUF_INTERRUPT

#define I2C_GPIO_PORT GPIOB
#define I2C_GPIO_SCL_PIN 8
#define I2C_GPIO_SDA_PIN 9

#define I2C_ENABLE_ACK(I2C)          ((I2C)->CR1 |= I2C_CR1_ACK)
#define I2C_DISABLE_ACK(I2C)         ((I2C)->CR1 &= ~I2C_CR1_ACK)
#define I2C_SEND_START_BIT(I2C)      ((I2C)->CR1 |= I2C_CR1_START)
#define I2C_WAIT_UNTIL_START_GENERATED(I2C) while (!((I2C)->SR1 & I2C_SR1_SB));
#define I2C_SEND_ADDR(I2C, addr, oper)     ((I2C)->DR = ((addr) << 1) | (oper))
#define I2C_SET_DATA_BYTE(I2C, byte) ((I2C)->DR = byte)
#define I2C_SEND_STOP_BIT(I2C)       ((I2C)->CR1 |= I2C_CR1_STOP)
#define I2C_CLEAR_ADDR(I2C) REG_DUMMY_READ((I2C)->SR1); REG_DUMMY_READ((I2C)->SR2);
#define I2C_ENABLE_EVT_INT(I2C) ((I2C)->CR2  |= I2C_CR2_ITEVTEN)
#define I2C_DISABLE_EVT_INT(I2C) ((I2C)->CR2 &= ~I2C_CR2_ITEVTEN)

typedef enum mh_i2c_state {
    MH_I2C_STATE_READY,
    MH_I2C_STATE_SENDING_ADDR_TRANSMIT,
    MH_I2C_STATE_WAIT_STARTB_RECEIVE,
    MH_I2C_STATE_SENDING_ADDR_RECEIVE,
    MH_I2C_STATE_TRANSMIT,
    MH_I2C_STATE_RECEIVE,
} MH_I2C_STATE_T;

#ifdef MH_DEBUG
static const char* const mh_i2c_state_str[] = {
    E_STR(MH_I2C_STATE_READY),
    E_STR(MH_I2C_STATE_SENDING_ADDR_TRANSMIT),
    E_STR(MH_I2C_STATE_SENDING_ADDR_RECEIVE),
    E_STR(MH_I2C_STATE_TRANSMIT),
    E_STR(MH_I2C_STATE_RECEIVE),
};
#endif

typedef enum mh_i2c_event {
    MH_I2C_EVENT_NONE,
    MH_I2C_EVENT_STARTB,
    MH_I2C_EVENT_ADDR10,
    MH_I2C_EVENT_ADDR,
    MH_I2C_EVENT_TX_READY,
    MH_I2C_EVENT_TX_DONE,
    MH_I2C_EVENT_RX_READY,
    MH_I2C_EVENT_RX_ACK,
} MH_I2C_EVENT_T;

typedef struct mh_i2c_data {
    uint32_t slave_addr;
    uint32_t reg;
    uint32_t result;
} MH_I2C_DATA;

static volatile MH_I2C_DATA i2c_data;
static void (*_i2c_handle_evt)(MH_I2C_EVENT_T event) = NULL;
static volatile MH_I2C_STATE_T state = MH_I2C_STATE_READY;

static inline void state_transition(MH_I2C_STATE_T new_state) {
    MH_LOGD_ISR("Transition state %s --> %s", mh_i2c_state_str[state], mh_i2c_state_str[new_state]);
    state = new_state;
}

__attribute__((weak)) void i2c_master_read_complete(uint32_t value) {
    UNUSED(value);
    //Do nothing
}

static MH_I2C_EVENT_T _i2c_parse_irq_event(void) {
    uint32_t SR1 = READ_REG(I2C1->SR1);
    uint32_t SR2 = READ_REG(I2C1->SR2);

    MH_I2C_EVENT_T event = MH_I2C_EVENT_NONE;

#ifdef ENABLE_EVT_INTERRUPT
    if (SR1 & I2C_SR1_SB) {
        event = MH_I2C_EVENT_STARTB;
    } else if (SR1 & I2C_SR1_ADD10) {
        event = MH_I2C_EVENT_ADDR10;
    } else if (SR1 & I2C_SR1_ADDR) {
        event = MH_I2C_EVENT_ADDR;
    }
#endif

    /* I2C in mode Transmitter -*/
    if (SR2 & I2C_SR2_TRA) {
#ifdef ENABLE_BUF_INTERRUPT
        if ((SR1 & I2C_SR1_TXE) && !(SR1 & I2C_SR1_BTF)) {
            event = MH_I2C_EVENT_TX_READY;
        } else
#endif
#ifdef ENABLE_EVT_INTERRUPT
        if (SR1 & I2C_SR1_BTF) {
            event = MH_I2C_EVENT_TX_DONE;
        }
#endif
    } else {
#ifdef ENABLE_BUF_INTERRUPT
        if ((SR1 & I2C_SR1_RXNE) && !(SR1 & I2C_SR1_BTF)) {
            event = MH_I2C_EVENT_RX_ACK;
        } else
#endif
#ifdef ENABLE_EVT_INTERRUPT
        if (SR1 & I2C_SR1_BTF) {
            event = MH_I2C_EVENT_RX_READY;
        }
#endif
    }
    return event;
}

// 7 bit one byte register read
// Send addr -> reg address -> addr -> get reg value
static void _i2c1_read_reg_evt_hdl(MH_I2C_EVENT_T event) {
    switch (event) {
#ifdef ENABLE_EVT_INTERRUPT
        case MH_I2C_EVENT_STARTB:
            switch (state) {
                case MH_I2C_STATE_READY:
                    I2C_SEND_ADDR(I2C1, i2c_data.slave_addr, false);
                    state_transition(MH_I2C_STATE_SENDING_ADDR_TRANSMIT);
                    break;
                case MH_I2C_STATE_WAIT_STARTB_RECEIVE:
                    I2C_SEND_ADDR(I2C1, i2c_data.slave_addr, true);
                    state_transition(MH_I2C_STATE_SENDING_ADDR_RECEIVE);
                    break;
                default:
                    break;
            }
            break;
        case MH_I2C_EVENT_ADDR:
            switch (state) {
                case MH_I2C_STATE_SENDING_ADDR_TRANSMIT:
                    I2C_SET_DATA_BYTE(I2C1, i2c_data.reg);

                    state_transition(MH_I2C_STATE_TRANSMIT);
                    break;
                case MH_I2C_STATE_SENDING_ADDR_RECEIVE:
                    I2C_DISABLE_ACK(I2C1);
                    I2C_CLEAR_ADDR(I2C1);
                    I2C_SEND_STOP_BIT(I2C1);

                    state_transition(MH_I2C_STATE_RECEIVE);
                default:
                    break;
            }
            break;
        case MH_I2C_EVENT_TX_DONE:
            switch (state) {
                case MH_I2C_STATE_TRANSMIT:
                    I2C_ENABLE_ACK(I2C1);
                    I2C_SEND_START_BIT(I2C1);
                    state_transition(MH_I2C_STATE_WAIT_STARTB_RECEIVE);
                    break;
                default:
                    break;
            }
            break;
        case MH_I2C_EVENT_RX_READY:
            switch (state) {
                case MH_I2C_STATE_RECEIVE:
                    i2c_data.result = I2C1->DR;
                    I2C_DISABLE_EVT_INT(I2C1);
                    i2c_master_read_complete(i2c_data.result);
                    state_transition(MH_I2C_STATE_READY);

                    break;
                default:
                    break;
            }
            break;
#endif
#ifdef ENABLE_BUF_INTERRUPT
        case MH_I2C_EVENT_RX_ACK:
            break;
        case MH_I2C_EVENT_TX_READY:
            break;
#endif
        default:
            break;
    }
}

void I2C1_EV_IRQHandler(void) {
    MH_I2C_EVENT_T event = _i2c_parse_irq_event();

    _i2c_handle_evt(event);
}

void i2c_init(unsigned master_addr, unsigned clock_rate) {
    uint32_t freqrange = I2C_FREQRANGE(APB1_CLOCK);
    uint32_t cr2_flags = I2C_CR2_ITERREN | freqrange;
    MH_GPIO_CONFIG gpio_config = { .mode = MH_GPIO_MODE_50MHZ, .direction = MH_GPIO_DIRECTION_AF_OD };

    MH_ASSERT(master_addr <= 0xff);

#ifdef ENABLE_BUF_INTERRUPT
    cr2_flags |= I2C_CR2_ITBUFEN;
#endif

    MH_LOGD("APBClock=%d", APB1_CLOCK);

    //GPIO AFIO APB
    RCC->APB2ENR |= (RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN);
    REG_DUMMY_READ(RCC->APB2ENR);
    //I2C APB
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    REG_DUMMY_READ(RCC->APB1ENR);

    NVIC_EnableIRQ(I2C1_EV_IRQn);
    //NVIC_EnableIRQ(I2C1_ER_IRQn);

    I2C1->CR2  |= cr2_flags;

    I2C1->TRISE = I2C_RISE_TIME(freqrange, clock_rate);
    I2C1->CCR  |= I2C_SPEED(APB1_CLOCK, clock_rate, I2C_DUTYCYCLE_2);
    I2C1->OAR1 |= (master_addr & ~I2C_OAR1_ADD0) | (1 << 14);
    I2C1->OAR2 |= 0xff & ~I2C_OAR1_ADD0;

    mh_gpio_pin_config(I2C_GPIO_PORT, I2C_GPIO_SCL_PIN, &gpio_config);
    mh_gpio_pin_config(I2C_GPIO_PORT, I2C_GPIO_SDA_PIN, &gpio_config);

    AFIO->MAPR |= AFIO_MAPR_I2C1_REMAP;

    I2C1->CR1  |= I2C_CR1_PE;
}

void i2c_master_read_reg_async(uint32_t slave_addr, uint32_t reg) {
    MH_ASSERT(slave_addr <= 0xff);
    MH_ASSERT(reg <= 0xff);

    i2c_data.reg = reg;
    i2c_data.slave_addr = slave_addr;

    _i2c_handle_evt = _i2c1_read_reg_evt_hdl;

#ifdef ENABLE_EVT_INTERRUPT
    I2C_ENABLE_EVT_INT(I2C1);
#endif
    I2C_SEND_START_BIT(I2C1);
}

void i2c_master_write_reg_async(uint32_t slave_addr, uint32_t reg, uint32_t value) {
    UNUSED(slave_addr); UNUSED(reg); UNUSED(value);
    //TODO: Not implemented
}

void i2c_deinit(void) {
    I2C1->CR1 &= ~I2C_CR1_PE;
}
