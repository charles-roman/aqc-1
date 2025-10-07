################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/flight/attitude.c \
../Core/Src/flight/failsafe.c \
../Core/Src/flight/mixer.c \
../Core/Src/flight/pid.c \
../Core/Src/flight/system.c 

OBJS += \
./Core/Src/flight/attitude.o \
./Core/Src/flight/failsafe.o \
./Core/Src/flight/mixer.o \
./Core/Src/flight/pid.o \
./Core/Src/flight/system.o 

C_DEPS += \
./Core/Src/flight/attitude.d \
./Core/Src/flight/failsafe.d \
./Core/Src/flight/mixer.d \
./Core/Src/flight/pid.d \
./Core/Src/flight/system.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/flight/%.o Core/Src/flight/%.su Core/Src/flight/%.cyclo: ../Core/Src/flight/%.c Core/Src/flight/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F405xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-flight

clean-Core-2f-Src-2f-flight:
	-$(RM) ./Core/Src/flight/attitude.cyclo ./Core/Src/flight/attitude.d ./Core/Src/flight/attitude.o ./Core/Src/flight/attitude.su ./Core/Src/flight/failsafe.cyclo ./Core/Src/flight/failsafe.d ./Core/Src/flight/failsafe.o ./Core/Src/flight/failsafe.su ./Core/Src/flight/mixer.cyclo ./Core/Src/flight/mixer.d ./Core/Src/flight/mixer.o ./Core/Src/flight/mixer.su ./Core/Src/flight/pid.cyclo ./Core/Src/flight/pid.d ./Core/Src/flight/pid.o ./Core/Src/flight/pid.su ./Core/Src/flight/system.cyclo ./Core/Src/flight/system.d ./Core/Src/flight/system.o ./Core/Src/flight/system.su

.PHONY: clean-Core-2f-Src-2f-flight

