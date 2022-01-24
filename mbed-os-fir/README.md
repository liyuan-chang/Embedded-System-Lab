# FIR Lowpass Filter

Process the sensor data of 3D accelerators and gyroscopes from STM32 IoT Node development board

# Usage

1. Modify `mbed-os/targets/targets.json`

   * printf_lib: "std"

2. Add **#define __CC_ARM** in `mbed-dsp/cmsis_dsp/TransformFunctions/arm_bitreversal2.S`

3. Compile and run the program on a STM board
