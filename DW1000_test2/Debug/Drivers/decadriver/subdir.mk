################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/decadriver/deca_device.c \
../Drivers/decadriver/deca_params_init.c 

OBJS += \
./Drivers/decadriver/deca_device.o \
./Drivers/decadriver/deca_params_init.o 

C_DEPS += \
./Drivers/decadriver/deca_device.d \
./Drivers/decadriver/deca_params_init.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/decadriver/%.o: ../Drivers/decadriver/%.c Drivers/decadriver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/mille/STM32CubeIDE/workspace_1.8.0/DW1000_test2/Drivers/decadriver" -I"C:/Users/mille/STM32CubeIDE/workspace_1.8.0/DW1000_test2/Core/Src/platform" -I"C:/Users/mille/STM32CubeIDE/workspace_1.8.0/DW1000_test2/Core/Src/compiler" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-decadriver

clean-Drivers-2f-decadriver:
	-$(RM) ./Drivers/decadriver/deca_device.d ./Drivers/decadriver/deca_device.o ./Drivers/decadriver/deca_params_init.d ./Drivers/decadriver/deca_params_init.o

.PHONY: clean-Drivers-2f-decadriver

