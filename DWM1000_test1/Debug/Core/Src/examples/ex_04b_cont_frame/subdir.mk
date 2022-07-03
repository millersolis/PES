################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/examples/ex_04b_cont_frame/main.c 

OBJS += \
./Core/Src/examples/ex_04b_cont_frame/main.o 

C_DEPS += \
./Core/Src/examples/ex_04b_cont_frame/main.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/examples/ex_04b_cont_frame/%.o: ../Core/Src/examples/ex_04b_cont_frame/%.c Core/Src/examples/ex_04b_cont_frame/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103x6 -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-examples-2f-ex_04b_cont_frame

clean-Core-2f-Src-2f-examples-2f-ex_04b_cont_frame:
	-$(RM) ./Core/Src/examples/ex_04b_cont_frame/main.d ./Core/Src/examples/ex_04b_cont_frame/main.o

.PHONY: clean-Core-2f-Src-2f-examples-2f-ex_04b_cont_frame

