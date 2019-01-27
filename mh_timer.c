#include "mh_timer.h"
#include "mh_utils.h"
#include "mh_event.h"
#include "mh_sleep.h"
#include "stm32f1xx.h"

#define PRESCALER_VALUE 65535UL

typedef void (*MH_TIMER_HDL)(void);

static void _idle_timer_hdl(void);

static MH_TIMER_T running_timer;

static const MH_TIMER_HDL _timer_handlers[MH_TIMER_MAX] = {
    _idle_timer_hdl,
};

static void _idle_timer_hdl(void) {
    MH_EVENT_T event = {.type = MH_EVENT_WAKEUP };
    mh_event_put(&event);
}

void TIM2_IRQHandler(void) {
    if (!(TIM2->SR & TIM_SR_UIF))
        return;
    _timer_handlers[running_timer]();
    TIM2->SR &= ~TIM_SR_UIF;
}

void mh_timer_init(void) {
    //Timer 2 APB
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    REG_DUMMY_READ(RCC->APB1ENR);

    NVIC_EnableIRQ(TIM2_IRQn);

    TIM2->DIER |= TIM_DIER_UIE;
    TIM2->CR1 |= TIM_CR1_OPM | TIM_CR1_URS ; //one-pulse mode, only over/underflow interrupt
    TIM2->PSC = PRESCALER_VALUE;
}

void mh_timer_start(MH_TIMER_T type, uint32_t timeout) {
    running_timer = type;
    TIM2->ARR = ((uint16_t) timeout) * (APB1_CLOCK / PRESCALER_VALUE / 2); // Divide APB clock by 2
    TIM2->SR = 0;
    TIM2->EGR |= TIM_EGR_UG;
    TIM2->CR1 |= TIM_CR1_CEN; // Enable counter
}

void mh_timer_deinit(void) {
}
