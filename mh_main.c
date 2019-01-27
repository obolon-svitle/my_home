#include <stdio.h>

#include "stm32f1xx.h"

#include "mh_state_machine.h"
#include "mh_timer.h"

#include "mh_sensor.h"
#include "mh_utils.h"
#include "mh_log.h"
#include "mh_transmitter.h"

static void system_clock_setup(void) {
    // Set system clock driver by 4xPLL * 8 Mhz / 2 HSI = 16Mhz.
    // APB1 freq 16 Mhz
    // PLL driver system clock
    RCC->CFGR |= RCC_CFGR_PLLMULL2 | RCC_CFGR_SW_PLL | RCC_CFGR_PPRE1_DIV16;

    //Enable HSI oscillator and set pll as sysclk
    RCC->CR |= (RCC_CR_HSION | RCC_CR_PLLON);

    // Enable PLL INT and wait for ready
    RCC->CIR |= RCC_CIR_PLLRDYIE;
    while (!(RCC->CIR & RCC_CIR_PLLRDYF)) ;

    SystemCoreClockUpdate();

    MH_LOGD("SystemCoreClock=%d", SystemCoreClock);
}

static void mh_modules_init(void) {
    mh_sensor_init();
    mh_transmitter_init();
    mh_timer_init();
    mh_state_machine_deinit();
}

static void mh_modules_deinit(void) {
    mh_state_machine_init();
    mh_timer_init();
    mh_transmitter_deinit();
    mh_sensor_deinit();
}

int main() {
    system_clock_setup();
    mh_modules_init();

    mh_state_machine_run();

    mh_modules_deinit();

    return 0;
}
