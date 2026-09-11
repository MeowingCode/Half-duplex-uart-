################################################################################
# MRS Version: 2.3.0
# Automatically-generated file. Do not edit!
################################################################################
-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include User/Modules/subdir.mk
-include User/subdir.mk
-include StdPeriphDriver/subdir.mk
-include Startup/subdir.mk
-include RVMSIS/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
UART_test.hex \

SECONDARY_LIST += \
UART_test.lst \

SECONDARY_SIZE += \
UART_test.siz \


# All Target
all: 
	$(MAKE) --no-print-directory main-build 

main-build: UART_test.elf secondary-outputs

# Tool invocations
UART_test.elf: $(OBJS) $(USER_OBJS_ESCAPE)
	@	riscv-none-embed-g++ -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -T "c:/Users/Jenny/mounriver-studio-projects/UART_test/Ld/Link.ld" -nostartfiles -Xlinker --gc-sections -L"c:\Users\Jenny\mounriver-studio-projects\UART_test" -L"c:/Users/Jenny/mounriver-studio-projects/UART_test/StdPeriphDriver" -Xlinker --print-memory-usage -Wl,-Map,"UART_test.map" --specs=nano.specs --specs=nosys.specs -o "UART_test.elf" $(OBJS) $(LIBS)
UART_test.hex: UART_test.elf
	@	riscv-none-embed-objcopy -O ihex "UART_test.elf" "UART_test.hex"
UART_test.lst: UART_test.elf
	@	riscv-none-embed-objdump --source --all-headers --demangle --line-numbers --wide "UART_test.elf" > "UART_test.lst"
UART_test.siz: UART_test.elf
	riscv-none-embed-size --format=berkeley "UART_test.elf"

# Other Targets
clean:
	-$(RM) $(DIR_OBJS) $(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE) UART_test.elf
	-$(RM) $(DIR_EXPANDS) $(CALLGRAPH_DOT)
	-$(RM) $(DIR_DEPS)
secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets