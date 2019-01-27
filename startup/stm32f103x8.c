#include <string.h>
#include <stdint.h>

#include "mh_utils.h"

typedef enum isr_num {
    ISR_NUM_STACK_PTR,
    ISR_NUM_RESET,
    ISR_NUM_NMI,
    ISR_NUM_HARDFAULT,
    ISR_NUM_MEMMANAGE,
    ISR_NUM_BUSFAULT,
    ISR_NUM_USAGEFAULT,
    /* 4 Reserved_vectors */
    ISR_NUM_SVC = ISR_NUM_USAGEFAULT + 5,
    ISR_NUM_DEBUGMON,
    /* 1 Reserved vector */
    ISR_NUM_PENDSV = ISR_NUM_DEBUGMON + 2,
    ISR_NUM_SYSTICK,
    ISR_NUM_WWDG,
    ISR_NUM_PVD,
    ISR_NUM_TAMPER,
    ISR_NUM_RTC,
    ISR_NUM_FLASH,
    ISR_NUM_RCC,
    ISR_NUM_EXTI0,
    ISR_NUM_EXTI1,
    ISR_NUM_EXTI2,
    ISR_NUM_EXTI3,
    ISR_NUM_EXTI4,
    ISR_NUM_DMA1_CHANNEL1,
    ISR_NUM_DMA1_CHANNEL2,
    ISR_NUM_DMA1_CHANNEL3,
    ISR_NUM_DMA1_CHANNEL4,
    ISR_NUM_DMA1_CHANNEL5,
    ISR_NUM_DMA1_CHANNEL6,
    ISR_NUM_DMA1_CHANNEL7,
    ISR_NUM_ADC1_2,
    ISR_NUM_USB_HP_CAN1_TX,
    ISR_NUM_USB_HP_CAN1_RX0,
    ISR_NUM_CAN1_RX1,
    ISR_NUM_CAN1_SCE,
    ISR_NUM_EXTI9_5,
    ISR_NUM_TIM1_BRK,
    ISR_NUM_TIM1_UP,
    ISR_NUM_TIM1_TRG_COM,
    ISR_NUM_TIM1_CC,
    ISR_NUM_TIM2,
    ISR_NUM_TIM3,
    ISR_NUM_TIM4,
    ISR_NUM_I2C1_EV,
    ISR_NUM_I2C1_ER,
    ISR_NUM_I2C2_EV,
    ISR_NUM_I2C2_ER,
    ISR_NUM_SPI1,
    ISR_NUM_SPI2,
    ISR_NUM_USART1,
    ISR_NUM_USART2,
    ISR_NUM_USART3,
    ISR_NUM_EXTI15_10,
    ISR_NUM_RTC_ALARM,
    ISR_NUM_USBWAKEUP,
    /* 7 reserved vectors */
    ISR_NUM_BOOTRAM = ISR_NUM_USBWAKEUP + 8,

    ISR_NUM_MAX,
} ISR_NUM;

#define BOOT_FROM_RAM_MAGIC ((isr_hdl_t)0xF108F85F)
#define STACK_SIZE 0x400

typedef void (* isr_hdl_t)(void);

void SystemInit(void);
int main(void);
void TIM2_IRQHandler(void);
void I2C1_EV_IRQHandler(void);

extern uint32_t _bss, _ebss;
extern uint32_t _data_loadaddr, _data, _edata;

__attribute__ ((section(".sys_stack")))
static uint32_t system_stack[STACK_SIZE];

void *memset(void *s, int c, size_t n) {
    char *p = s;
    while (n--) {
        *((char *) p) = c;
        p++;
    }

    return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
    char *d = dest;
    const char *s = src;

    while (n--) {
        *(d++) = *(s++);
    }

    return dest;
}

static void _get_regs(uint32_t *stk) {
    volatile uint32_t r0, r1, r2, r3, r12, lr, pc, psr;
    r0  = stk[0];
    r1  = stk[1];
    r2  = stk[2];
    r3  = stk[3];
    r12 = stk[4];
    lr  = stk[5];
    pc  = stk[6];
    psr = stk[7];

    UNUSED(r0); UNUSED(r1); UNUSED(r2); UNUSED(r3); UNUSED(r12); UNUSED(lr);
    UNUSED(pc); UNUSED(psr);

    for (;;)
        ;
}

static void _fault_handler(void) __attribute__((naked));
static void _fault_handler(void) {
    __asm volatile (
        " tst lr, #4 \n\t"
        " ite eq \n\t"
        " mrseq r0, msp \n\t"
        " mrsne r0, psp \n\t"
        " ldr r1, [r0, #24] \n\t"
        " ldr r2, handler2_address_const \n\t"
        " bx r2 \n\t"
        " handler2_address_const: .word _get_regs \n\t"
    );

    UNUSED(_get_regs);
}

static void _default_handler(void) {
    for (;;)
        ;
}

void reset_handler(void) {
    uint32_t *p = &_bss;
    uint32_t *d;

    while (p != &_ebss) {
        *(p++) = 0;
    }

    p = &_data;
    d = &_data_loadaddr;

    while (p != &_edata) {
        *(p++) = *(d++);
    }

    SystemInit();
    main();

    for (;;);
}

__attribute__ ((section(".isr_vector")))
const isr_hdl_t g_pfnVectors[ISR_NUM_MAX] = {
    [ISR_NUM_STACK_PTR]       = (void (*)(void))(((unsigned long)&system_stack) + sizeof(system_stack)),
    [ISR_NUM_RESET]           = reset_handler,
    [ISR_NUM_NMI]             = _default_handler,
    [ISR_NUM_HARDFAULT]       = _fault_handler,
    [ISR_NUM_MEMMANAGE]       = _default_handler,
    [ISR_NUM_BUSFAULT]        = _default_handler,
    [ISR_NUM_USAGEFAULT]      = _default_handler,
    [ISR_NUM_SVC]             = _default_handler,
    [ISR_NUM_DEBUGMON]        = _default_handler,
    [ISR_NUM_PENDSV]          = _default_handler,
    [ISR_NUM_SYSTICK]         = _default_handler,
    [ISR_NUM_WWDG]            = _default_handler,
    [ISR_NUM_PVD]             = _default_handler,
    [ISR_NUM_TAMPER]          = _default_handler,
    [ISR_NUM_RTC]             = _default_handler,
    [ISR_NUM_FLASH]           = _default_handler,
    [ISR_NUM_RCC]             = _default_handler,
    [ISR_NUM_EXTI0]           = _default_handler,
    [ISR_NUM_EXTI1]           = _default_handler,
    [ISR_NUM_EXTI2]           = _default_handler,
    [ISR_NUM_EXTI3]           = _default_handler,
    [ISR_NUM_EXTI4]           = _default_handler,
    [ISR_NUM_DMA1_CHANNEL1]   = _default_handler,
    [ISR_NUM_DMA1_CHANNEL2]   = _default_handler,
    [ISR_NUM_DMA1_CHANNEL3]   = _default_handler,
    [ISR_NUM_DMA1_CHANNEL4]   = _default_handler,
    [ISR_NUM_DMA1_CHANNEL5]   = _default_handler,
    [ISR_NUM_DMA1_CHANNEL6]   = _default_handler,
    [ISR_NUM_DMA1_CHANNEL7]   = _default_handler,
    [ISR_NUM_ADC1_2]          = _default_handler,
    [ISR_NUM_USB_HP_CAN1_TX]  = _default_handler,
    [ISR_NUM_USB_HP_CAN1_RX0] = _default_handler,
    [ISR_NUM_CAN1_RX1]        = _default_handler,
    [ISR_NUM_CAN1_SCE]        = _default_handler,
    [ISR_NUM_EXTI9_5]         = _default_handler,
    [ISR_NUM_TIM1_BRK]        = _default_handler,
    [ISR_NUM_TIM1_UP]         = _default_handler,
    [ISR_NUM_TIM1_TRG_COM]    = _default_handler,
    [ISR_NUM_TIM1_CC]         = _default_handler,
    [ISR_NUM_TIM2]            = TIM2_IRQHandler,
    [ISR_NUM_TIM3]            = _default_handler,
    [ISR_NUM_TIM4]            = _default_handler,
    [ISR_NUM_I2C1_EV]         = I2C1_EV_IRQHandler,
    [ISR_NUM_I2C1_ER]         = _default_handler,
    [ISR_NUM_I2C2_EV]         = _default_handler,
    [ISR_NUM_I2C2_ER]         = _default_handler,
    [ISR_NUM_SPI1]            = _default_handler,
    [ISR_NUM_SPI2]            = _default_handler,
    [ISR_NUM_USART1]          = _default_handler,
    [ISR_NUM_USART2]          = _default_handler,
    [ISR_NUM_USART3]          = _default_handler,
    [ISR_NUM_EXTI15_10]       = _default_handler,
    [ISR_NUM_RTC_ALARM]       = _default_handler,
    [ISR_NUM_USBWAKEUP]       = _default_handler,

    [ISR_NUM_BOOTRAM]         = BOOT_FROM_RAM_MAGIC,
};
