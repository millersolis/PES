################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/decadriver/deca_device.c \
../Core/Src/decadriver/deca_params_init.c 

OBJS += \
./Core/Src/decadriver/deca_device.o \
./Core/Src/decadriver/deca_params_init.o 

C_DEPS += \
./Core/Src/decadriver/deca_device.d \
./Core/Src/decadriver/deca_params_init.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/decadriver/%.o: ../Core/Src/decadriver/%.c Core/Src/decadriver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103x6 -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-decadriver

clean-Core-2f-Src-2f-decadriver:
	-$(RM) ./Core/Src/decadriver/deca_device.d ./Core/Src/decadriver/deca_device.o ./Core/Src/decadriver/deca_params_init.d ./Core/Src/decadriver/deca_params_init.o

.PHONY: clean-Core-2f-Src-2f-decadriver

