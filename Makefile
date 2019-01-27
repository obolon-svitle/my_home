COMPILER_PREFIX ?= arm-none-eabi-
CC               = $(COMPILER_PREFIX)gcc
LD               = $(COMPILER_PREFIX)ld
OBJCOPY          = $(COMPILER_PREFIX)objcopy

BOARD_NAME      := stm32f103x8
STM32CUBE_PATH   = third_party/STM32Cube_FW_F1_V1.7.0/

LINKER_SCRIPT = linker/$(BOARD_NAME).ld

SRCS     = mh_main.c

SRCS    += startup/$(BOARD_NAME).c
SRCS    += mh_state_machine.c
SRCS    += mh_gpio.c
SRCS    += mh_sensor.c
SRCS    += mh_timer.c
SRCS    += mh_i2c.c
SRCS    += mh_transmitter.c

CFLAGS  += -mcpu=cortex-m3 -std=gnu99 -pedantic -mthumb
CFLAGS  += -ffreestanding

CFLAGS  += -g3 -O0
CFLAGS  += -Wall -Wextra -Werror -Wfatal-errors
CFLAGS  += -Iinc
CFLAGS  += -DMH_DEBUG

LDFLAGS += -nostartfiles -nodefaultlibs -nostdlib --gc-sections
LDFLAGS += -L linker -T$(LINKER_SCRIPT)
LDFLAGS += --fatal-warnings

# -> CMSIS
SRCS_CMSIS    = $(STM32CUBE_PATH)/Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/system_stm32f1xx.c
CFLAGS_CMSIS  = -I$(STM32CUBE_PATH)/Drivers/CMSIS/Include
CFLAGS_CMSIS += -I$(STM32CUBE_PATH)/Drivers/CMSIS/Device/ST/STM32F1xx/Include/
CFLAGS_CMSIS += -DSTM32F103xB

SRCS         += $(SRCS_CMSIS)
CFLAGS       += $(CFLAGS_CMSIS)
# <- CMSIS

# -> STM32 HAL
CFLAGS_HAL += -I$(STM32CUBE_PATH)/Drivers/STM32F1xx_HAL_Driver/Inc/

CFLAGS     += $(CFLAGS_HAL)
# <- STM32 HAL

OUT_DIR = out
OBJS   := $(addprefix $(OUT_DIR)/, $(SRCS:.c=.o))

all: $(BOARD_NAME).bin

ifeq ($(wildcard $(STM32CUBE_PATH)/.*),)
	$(error No SDK dir. Download it at \
	https://www.st.com/content/st_com/en/products/embedded-software/mcus-embedded-software/stm32-embedded-software/stm32cube-mcu-packages/stm32cubef1.html#sw-tools-scroll \
	and unpack into "$(STM32CUBE_PATH)" folder)
endif

$(BOARD_NAME).bin : $(BOARD_NAME).elf
	$(OBJCOPY) $^ $@ 
	
$(BOARD_NAME).elf: $(OBJS) $(LINKER_SCRIPT)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

$(OBJS) : $(OUT_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean

clean:
	rm -rf $(BOARD_NAME).bin $(BOARD_NAME).elf $(OUT_DIR)
