################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32L4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/platform/subdir.mk
-include Core/Src/examples/ex_10a_gpio/subdir.mk
-include Core/Src/examples/ex_09b_bandwidth_power_comp/subdir.mk
-include Core/Src/examples/ex_09a_bandwidth_power_ref_meas/subdir.mk
-include Core/Src/examples/ex_07b_ack_data_rx/subdir.mk
-include Core/Src/examples/ex_07a_ack_data_tx/subdir.mk
-include Core/Src/examples/ex_06b_ss_twr_resp/subdir.mk
-include Core/Src/examples/ex_06a_ss_twr_init/subdir.mk
-include Core/Src/examples/ex_05b_ds_twr_resp/subdir.mk
-include Core/Src/examples/ex_05a_ds_twr_init/subdir.mk
-include Core/Src/examples/ex_04b_cont_frame/subdir.mk
-include Core/Src/examples/ex_04a_cont_wave/subdir.mk
-include Core/Src/examples/ex_03d_tx_wait_resp_interrupts/subdir.mk
-include Core/Src/examples/ex_03c_tx_wait_resp_leds/subdir.mk
-include Core/Src/examples/ex_03b_rx_send_resp/subdir.mk
-include Core/Src/examples/ex_03a_tx_wait_resp/subdir.mk
-include Core/Src/examples/ex_02f_rx_with_crystal_trim/subdir.mk
-include Core/Src/examples/ex_02e_rx_dbl_buff/subdir.mk
-include Core/Src/examples/ex_02d_rx_sniff/subdir.mk
-include Core/Src/examples/ex_02c_rx_diagnostics/subdir.mk
-include Core/Src/examples/ex_02b_rx_preamble_64/subdir.mk
-include Core/Src/examples/ex_02a_simple_rx/subdir.mk
-include Core/Src/examples/ex_01e_tx_with_cca/subdir.mk
-include Core/Src/examples/ex_01d_tx_timed_sleep/subdir.mk
-include Core/Src/examples/ex_01c_tx_sleep_auto/subdir.mk
-include Core/Src/examples/ex_01b_tx_sleep/subdir.mk
-include Core/Src/examples/ex_01a_simple_tx/subdir.mk
-include Core/Src/decadriver/subdir.mk
-include Core/Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := aaa_2
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
aaa_2.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
aaa_2.list \

OBJCOPY_BIN += \
aaa_2.bin \


# All Target
all: main-build

# Main-build Target
main-build: aaa_2.elf secondary-outputs

# Tool invocations
aaa_2.elf: $(OBJS) $(USER_OBJS) C:\Users\mille\STM32CubeIDE\workspace_1.8.0\aaa_2\STM32L432KCUX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "aaa_2.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"C:\Users\mille\STM32CubeIDE\workspace_1.8.0\aaa_2\STM32L432KCUX_FLASH.ld" --specs=nosys.specs -Wl,-Map="aaa_2.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

aaa_2.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "aaa_2.list"
	@echo 'Finished building: $@'
	@echo ' '

aaa_2.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "aaa_2.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) aaa_2.bin aaa_2.elf aaa_2.list default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
