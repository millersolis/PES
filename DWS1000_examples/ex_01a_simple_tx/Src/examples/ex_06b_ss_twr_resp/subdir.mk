################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/examples/ex_06b_ss_twr_resp/ex_06b_main.c 

OBJS += \
./Src/examples/ex_06b_ss_twr_resp/ex_06b_main.o 

C_DEPS += \
./Src/examples/ex_06b_ss_twr_resp/ex_06b_main.d 


# Each subdirectory must supply rules for building sources it contributes
Src/examples/ex_06b_ss_twr_resp/%.o: ../Src/examples/ex_06b_ss_twr_resp/%.c Src/examples/ex_06b_ss_twr_resp/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F429xx -DEX_01A_DEF -c -I../Inc -I../Src/compiler -I../Src/decadriver -I../Src/platform -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O2 -ffunction-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@"  -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src-2f-examples-2f-ex_06b_ss_twr_resp

clean-Src-2f-examples-2f-ex_06b_ss_twr_resp:
	-$(RM) ./Src/examples/ex_06b_ss_twr_resp/ex_06b_main.d ./Src/examples/ex_06b_ss_twr_resp/ex_06b_main.o

.PHONY: clean-Src-2f-examples-2f-ex_06b_ss_twr_resp

