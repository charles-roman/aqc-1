################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/setup/setup.c 

OBJS += \
./Core/Src/setup/setup.o 

C_DEPS += \
./Core/Src/setup/setup.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/setup/%.o Core/Src/setup/%.su Core/Src/setup/%.cyclo: ../Core/Src/setup/%.c Core/Src/setup/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F405xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-setup

clean-Core-2f-Src-2f-setup:
	-$(RM) ./Core/Src/setup/setup.cyclo ./Core/Src/setup/setup.d ./Core/Src/setup/setup.o ./Core/Src/setup/setup.su

.PHONY: clean-Core-2f-Src-2f-setup

