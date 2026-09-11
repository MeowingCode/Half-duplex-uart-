################################################################################
# MRS Version: 2.3.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../User/Modules/Uart_Port.cpp 

CPP_DEPS += \
./User/Modules/Uart_Port.d 

OBJS += \
./User/Modules/Uart_Port.o 

DIR_OBJS += \
./User/Modules/*.o \

DIR_DEPS += \
./User/Modules/*.d \

DIR_EXPANDS += \
./User/Modules/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
User/Modules/%.o: ../User/Modules/%.cpp
	@	riscv-none-embed-g++ -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"c:/Users/Jenny/mounriver-studio-projects/UART_test/StdPeriphDriver/inc" -I"c:/Users/Jenny/mounriver-studio-projects/UART_test/RVMSIS" -std=gnu++11 -fabi-version=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

