################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/examples/ex_02d_rx_sniff/main.c 

OBJS += \
./Core/Src/examples/ex_02d_rx_sniff/main.o 

C_DEPS += \
./Core/Src/examples/ex_02d_rx_sniff/main.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/examples/ex_02d_rx_sniff/%.o: ../Core/Src/examples/ex_02d_rx_sniff/%.c Core/Src/examples/ex_02d_rx_sniff/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103x6 -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-examples-2f-ex_02d_rx_sniff

clean-Core-2f-Src-2f-examples-2f-ex_02d_rx_sniff:
	-$(RM) ./Core/Src/examples/ex_02d_rx_sniff/main.d ./Core/Src/examples/ex_02d_rx_sniff/main.o

.PHONY: clean-Core-2f-Src-2f-examples-2f-ex_02d_rx_sniff

