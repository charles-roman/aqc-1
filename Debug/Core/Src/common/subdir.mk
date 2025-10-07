################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/common/led.c \
../Core/Src/common/maths.c \
../Core/Src/common/time.c 

OBJS += \
./Core/Src/common/led.o \
./Core/Src/common/maths.o \
./Core/Src/common/time.o 

C_DEPS += \
./Core/Src/common/led.d \
./Core/Src/common/maths.d \
./Core/Src/common/time.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/common/%.o Core/Src/common/%.su Core/Src/common/%.cyclo: ../Core/Src/common/%.c Core/Src/common/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F405xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-common

clean-Core-2f-Src-2f-common:
	-$(RM) ./Core/Src/common/led.cyclo ./Core/Src/common/led.d ./Core/Src/common/led.o ./Core/Src/common/led.su ./Core/Src/common/maths.cyclo ./Core/Src/common/maths.d ./Core/Src/common/maths.o ./Core/Src/common/maths.su ./Core/Src/common/time.cyclo ./Core/Src/common/time.d ./Core/Src/common/time.o ./Core/Src/common/time.su

.PHONY: clean-Core-2f-Src-2f-common

