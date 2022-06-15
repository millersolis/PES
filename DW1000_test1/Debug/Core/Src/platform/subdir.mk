################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/platform/deca_mutex.c \
../Core/Src/platform/deca_range_tables.c \
../Core/Src/platform/deca_sleep.c \
../Core/Src/platform/deca_spi.c \
../Core/Src/platform/port.c \
../Core/Src/platform/stdio_d.c 

OBJS += \
./Core/Src/platform/deca_mutex.o \
./Core/Src/platform/deca_range_tables.o \
./Core/Src/platform/deca_sleep.o \
./Core/Src/platform/deca_spi.o \
./Core/Src/platform/port.o \
./Core/Src/platform/stdio_d.o 

C_DEPS += \
./Core/Src/platform/deca_mutex.d \
./Core/Src/platform/deca_range_tables.d \
./Core/Src/platform/deca_sleep.d \
./Core/Src/platform/deca_spi.d \
./Core/Src/platform/port.d \
./Core/Src/platform/stdio_d.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/platform/%.o: ../Core/Src/platform/%.c Core/Src/platform/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/mille/STM32CubeIDE/workspace_1.8.0/DW1000_test1/Drivers/decadriver" -I"C:/Users/mille/STM32CubeIDE/workspace_1.8.0/DW1000_test1/Core/Src/compiler" -I"C:/Users/mille/STM32CubeIDE/workspace_1.8.0/DW1000_test1/Core/Src/platform" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-platform

clean-Core-2f-Src-2f-platform:
	-$(RM) ./Core/Src/platform/deca_mutex.d ./Core/Src/platform/deca_mutex.o ./Core/Src/platform/deca_range_tables.d ./Core/Src/platform/deca_range_tables.o ./Core/Src/platform/deca_sleep.d ./Core/Src/platform/deca_sleep.o ./Core/Src/platform/deca_spi.d ./Core/Src/platform/deca_spi.o ./Core/Src/platform/port.d ./Core/Src/platform/port.o ./Core/Src/platform/stdio_d.d ./Core/Src/platform/stdio_d.o

.PHONY: clean-Core-2f-Src-2f-platform

