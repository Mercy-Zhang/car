################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Arm Compiler - building file: "$<"'
	"D:/ti/ccs2101/ccs/tools/compiler/ti-cgt-armllvm_5.1.1.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"D:/ti/code/car_tb6612/application" -I"D:/ti/code/car_tb6612" -I"D:/ti/code/car_tb6612/Debug" -I"D:/ti/ccs2101/mspm0_sdk_2_11_00_07/source/third_party/CMSIS/Core/Include" -I"D:/ti/ccs2101/mspm0_sdk_2_11_00_07/source" -I"D:/ti/code/car_tb6612/BSP/OLED" -I"D:/ti/code/car_tb6612/BSP/UART" -I"D:/ti/code/car_tb6612/BSP/key" -I"D:/ti/code/car_tb6612/BSP/delay" -g -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-1045784547: ../empty.syscfg
	@echo 'SysConfig - building file: "$<"'
	"D:/ti/ccs2101/sysconfig_1.26.2/sysconfig_cli.bat" -s "D:/ti/ccs2101/mspm0_sdk_2_11_00_07/.metadata/product.json" --script "D:/ti/code/car_tb6612/empty.syscfg" -o "." --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

device_linker.cmd: build-1045784547 ../empty.syscfg
device.opt: build-1045784547
device.cmd.genlibs: build-1045784547
ti_msp_dl_config.c: build-1045784547
ti_msp_dl_config.h: build-1045784547
Event.dot: build-1045784547

%.o: ./%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Arm Compiler - building file: "$<"'
	"D:/ti/ccs2101/ccs/tools/compiler/ti-cgt-armllvm_5.1.1.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"D:/ti/code/car_tb6612/application" -I"D:/ti/code/car_tb6612" -I"D:/ti/code/car_tb6612/Debug" -I"D:/ti/ccs2101/mspm0_sdk_2_11_00_07/source/third_party/CMSIS/Core/Include" -I"D:/ti/ccs2101/mspm0_sdk_2_11_00_07/source" -I"D:/ti/code/car_tb6612/BSP/OLED" -I"D:/ti/code/car_tb6612/BSP/UART" -I"D:/ti/code/car_tb6612/BSP/key" -I"D:/ti/code/car_tb6612/BSP/delay" -g -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

startup_mspm0g350x_ticlang.o: D:/ti/ccs2101/mspm0_sdk_2_11_00_07/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g350x_ticlang.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Arm Compiler - building file: "$<"'
	"D:/ti/ccs2101/ccs/tools/compiler/ti-cgt-armllvm_5.1.1.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"D:/ti/code/car_tb6612/application" -I"D:/ti/code/car_tb6612" -I"D:/ti/code/car_tb6612/Debug" -I"D:/ti/ccs2101/mspm0_sdk_2_11_00_07/source/third_party/CMSIS/Core/Include" -I"D:/ti/ccs2101/mspm0_sdk_2_11_00_07/source" -I"D:/ti/code/car_tb6612/BSP/OLED" -I"D:/ti/code/car_tb6612/BSP/UART" -I"D:/ti/code/car_tb6612/BSP/key" -I"D:/ti/code/car_tb6612/BSP/delay" -g -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


