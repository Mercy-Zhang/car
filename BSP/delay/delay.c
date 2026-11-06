#include "delay.h"

void delay_ms(uint32_t ms)
{
    // n = t/T = t*f ->1ms对应多少个周期
    uint32_t cycles = (CPUCLK_FREQ / 1000) * ms;
    delay_cycles(cycles);
}