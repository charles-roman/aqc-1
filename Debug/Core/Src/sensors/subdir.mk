################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/sensors/bar.c \
../Core/Src/sensors/gps.c \
../Core/Src/sensors/imu.c \
../Core/Src/sensors/mag.c 

OBJS += \
./Core/Src/sensors/bar.o \
./Core/Src/sensors/gps.o \
./Core/Src/sensors/imu.o \
./Core/Src/sensors/mag.o 

C_DEPS += \
./Core/Src/sensors/bar.d \
./Core/Src/sensors/gps.d \
./Core/Src/sensors/imu.d \
./Core/Src/sensors/mag.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/sensors/%.o Core/Src/sensors/%.su Core/Src/sensors/%.cyclo: ../Core/Src/sensors/%.c Core/Src/sensors/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F405xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-sensors

clean-Core-2f-Src-2f-sensors:
	-$(RM) ./Core/Src/sensors/bar.cyclo ./Core/Src/sensors/bar.d ./Core/Src/sensors/bar.o ./Core/Src/sensors/bar.su ./Core/Src/sensors/gps.cyclo ./Core/Src/sensors/gps.d ./Core/Src/sensors/gps.o ./Core/Src/sensors/gps.su ./Core/Src/sensors/imu.cyclo ./Core/Src/sensors/imu.d ./Core/Src/sensors/imu.o ./Core/Src/sensors/imu.su ./Core/Src/sensors/mag.cyclo ./Core/Src/sensors/mag.d ./Core/Src/sensors/mag.o ./Core/Src/sensors/mag.su

.PHONY: clean-Core-2f-Src-2f-sensors

