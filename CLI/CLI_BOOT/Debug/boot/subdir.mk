################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../boot/common.c \
../boot/flash_if.c \
../boot/menu.c \
../boot/xmodem.c \
../boot/ymodem.c 

OBJS += \
./boot/common.o \
./boot/flash_if.o \
./boot/menu.o \
./boot/xmodem.o \
./boot/ymodem.o 

C_DEPS += \
./boot/common.d \
./boot/flash_if.d \
./boot/menu.d \
./boot/xmodem.d \
./boot/ymodem.d 


# Each subdirectory must supply rules for building sources it contributes
boot/%.o boot/%.su boot/%.cyclo: ../boot/%.c boot/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F401xE -c -I"D:/STM32CUBE_IDE/CLI_BOOT/CLI" -I"D:/STM32CUBE_IDE/CLI_BOOT/boot" -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/Fish/Desktop/CLI/CLI_TEST/boot" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-boot

clean-boot:
	-$(RM) ./boot/common.cyclo ./boot/common.d ./boot/common.o ./boot/common.su ./boot/flash_if.cyclo ./boot/flash_if.d ./boot/flash_if.o ./boot/flash_if.su ./boot/menu.cyclo ./boot/menu.d ./boot/menu.o ./boot/menu.su ./boot/xmodem.cyclo ./boot/xmodem.d ./boot/xmodem.o ./boot/xmodem.su ./boot/ymodem.cyclo ./boot/ymodem.d ./boot/ymodem.o ./boot/ymodem.su

.PHONY: clean-boot

