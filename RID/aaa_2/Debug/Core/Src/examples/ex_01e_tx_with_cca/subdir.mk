################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/examples/ex_01e_tx_with_cca/ex_01e_tx_with_cca.c 

OBJS += \
./Core/Src/examples/ex_01e_tx_with_cca/ex_01e_tx_with_cca.o 

C_DEPS += \
./Core/Src/examples/ex_01e_tx_with_cca/ex_01e_tx_with_cca.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/examples/ex_01e_tx_with_cca/%.o: ../Core/Src/examples/ex_01e_tx_with_cca/%.c Core/Src/examples/ex_01e_tx_with_cca/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L432xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/mille/STM32CubeIDE/workspace_1.8.0/aaa_2/Core/Src/compiler" -I"C:/Users/mille/STM32CubeIDE/workspace_1.8.0/aaa_2/Core/Src/decadriver" -I"C:/Users/mille/STM32CubeIDE/workspace_1.8.0/aaa_2/Core/Src/platform" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-examples-2f-ex_01e_tx_with_cca

clean-Core-2f-Src-2f-examples-2f-ex_01e_tx_with_cca:
	-$(RM) ./Core/Src/examples/ex_01e_tx_with_cca/ex_01e_tx_with_cca.d ./Core/Src/examples/ex_01e_tx_with_cca/ex_01e_tx_with_cca.o

.PHONY: clean-Core-2f-Src-2f-examples-2f-ex_01e_tx_with_cca

