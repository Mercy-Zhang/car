################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
BSP/key/%.o: ../BSP/key/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Arm Compiler - building file: "$<"'
	"D:/ti/ccs2101/ccs/tools/compiler/ti-cgt-armllvm_5.1.1.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"D:/CCS/car_tb6612/car_tb6612/application" -I"D:/CCS/car_tb6612/car_tb6612" -I"D:/CCS/car_tb6612/car_tb6612/Debug" -I"C:/TI/mspm0_sdk_2_11_00_07/source/third_party/CMSIS/Core/Include" -I"C:/TI/mspm0_sdk_2_11_00_07/source" -I"D:/CCS/car_tb6612/car_tb6612/BSP/OLED" -I"D:/CCS/car_tb6612/car_tb6612/BSP/UART" -I"D:/CCS/car_tb6612/car_tb6612/BSP/key" -I"D:/CCS/car_tb6612/car_tb6612/BSP/MPU6050" -I"D:/CCS/car_tb6612/car_tb6612/BSP/delay" -g -Wall -MMD -MP -MF"BSP/key/$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


