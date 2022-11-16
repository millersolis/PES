################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: gcc
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/examples/ex_02b_rx_preamble_64/ex_02b_main.c 

OBJS += \
./Core/examples/ex_02b_rx_preamble_64/ex_02b_main.o 

C_DEPS += \
./Core/examples/ex_02b_rx_preamble_64/ex_02b_main.d 


# Each subdirectory must supply rules for building sources it contributes
Core/examples/ex_02b_rx_preamble_64/%.o Core/examples/ex_02b_rx_preamble_64/%.su: ../Core/examples/ex_02b_rx_preamble_64/%.c Core/examples/ex_02b_rx_preamble_64/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F303xE -c -I../Core/Inc -I../Core/examples -I../Core/Src/compiler -I../Core/Src/decadriver -I../Core/Src/platform -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-examples-2f-ex_02b_rx_preamble_64

clean-Core-2f-examples-2f-ex_02b_rx_preamble_64:
	-$(RM) ./Core/examples/ex_02b_rx_preamble_64/ex_02b_main.d ./Core/examples/ex_02b_rx_preamble_64/ex_02b_main.o ./Core/examples/ex_02b_rx_preamble_64/ex_02b_main.su

.PHONY: clean-Core-2f-examples-2f-ex_02b_rx_preamble_64

