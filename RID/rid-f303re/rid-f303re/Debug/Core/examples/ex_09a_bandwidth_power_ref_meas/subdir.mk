################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: gcc
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/examples/ex_09a_bandwidth_power_ref_meas/ex_09a_main.c 

OBJS += \
./Core/examples/ex_09a_bandwidth_power_ref_meas/ex_09a_main.o 

C_DEPS += \
./Core/examples/ex_09a_bandwidth_power_ref_meas/ex_09a_main.d 


# Each subdirectory must supply rules for building sources it contributes
Core/examples/ex_09a_bandwidth_power_ref_meas/%.o Core/examples/ex_09a_bandwidth_power_ref_meas/%.su: ../Core/examples/ex_09a_bandwidth_power_ref_meas/%.c Core/examples/ex_09a_bandwidth_power_ref_meas/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F303xE -c -I../Core/Inc -I../Core/examples -I../Core/Src/compiler -I../Core/Src/decadriver -I../Core/Src/platform -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-examples-2f-ex_09a_bandwidth_power_ref_meas

clean-Core-2f-examples-2f-ex_09a_bandwidth_power_ref_meas:
	-$(RM) ./Core/examples/ex_09a_bandwidth_power_ref_meas/ex_09a_main.d ./Core/examples/ex_09a_bandwidth_power_ref_meas/ex_09a_main.o ./Core/examples/ex_09a_bandwidth_power_ref_meas/ex_09a_main.su

.PHONY: clean-Core-2f-examples-2f-ex_09a_bandwidth_power_ref_meas

