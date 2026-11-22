/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ti_msp_dl_config.h"
#include "delay.h"
#include "oled.h"
#include "uart.h"
#include "key.h"
#include "motor.h"
#include "huidu.h"
#include "mpu_port.h"


//外部声明 mpu_port.c 里的时间戳变量
extern volatile uint32_t sys_tick_ms;

//SysTick 中断服务函数（每 1ms 触发一次）
void SysTick_Handler(void){
    sys_tick_ms++;
}

#define UI_PERIOD_MS        200U        /* 显示 / 串口上报周期（按主循环次数近似计时）*/

/* 极简的无符号整数打印，避免把 printf 拉进来 */
static void uart_send_u32(uint32_t v)
{
    char    buf[10];
    uint8_t n = 0U;

    if (v == 0U) {
        UART_Send_char(PRINT_INST, (uint8_t)'0');
        return;
    }
    while ((v > 0U) && (n < sizeof(buf))) {
        buf[n++] = (char)('0' + (v % 10U));
        v /= 10U;
    }
    while (n > 0U) {
        n--;
        UART_Send_char(PRINT_INST, (uint8_t)buf[n]);
    }
}

/* 五路灰度：按 huidu_value[] 的顺序（L1 L2 M R1 R2）直接发 5 个 0/1 字符，不引入 sprintf */
static void uart_send_huidu(void)
{
    uint8_t i;

    for (i = 0U; i < 5U; i++) {
        UART_Send_char(PRINT_INST, (uint8_t)('0' + huidu_value[i]));
    }
    UART_Send_char(PRINT_INST, (uint8_t)'\n');
}

int main(void)
{
    uint8_t  running  = 1U;     /* 0：停止；1：闭环运行（按键逻辑暂时关闭，上电即运行）*/
    //uint16_t key_lock = 0U;     /* 按键锁定计数，防止抖动连发 */
    uint32_t ui_tick  = 0U;

    float pitch = 0, roll = 0, yaw = 0;

    SYSCFG_DL_init();

    OLED_Init();
    OLED_ColorTurn(0);
    OLED_DisplayTurn(0);
    OLED_Clear();

    while(DMP_Init());

    //KEY_Init();
    UART_DMA_RX_Init();
    motor_init();                       /* 两轮 + 编码器中断 + TIM_MOTOR(100Hz) 控制环 */
    motor_set_target_speed_both(200.0f);/* 上电先让两轮都跑 200mm/s，按键以后再说 */

    while (1) {
        /* ---------- 按键：启动 / 停止（暂时不管） ----------
         * 以后恢复时，在键按下处做：
         *     running = (running != 0U) ? 0U : 1U;
         *     motor_set_target_speed_both(running ? 200.0f : 0.0f);
         *     key_lock = 200U;
         */

        /* ---------- UART：收到一整帧后回显 ----------
         * DMA 已经在 UART 中断里重新装填，这里不要再调用 UART_DMA_RX_Rearm()，
         * 否则会把正在接收的下一帧打断。
         */
        // if (uart_rx_done) {
        //     uint8_t  frame[UART_RX_FRAME_SIZE];
        //     uint32_t i;

        //     uart_rx_done = false;
        //     for (i = 0U; i < UART_RX_FRAME_SIZE; i++) {
        //         frame[i] = uart_rx_buf[i];          /* 先拷出来，避免回显时被 DMA 改写 */
        //     }

        //     /* TODO: 在这里解析 frame[] 设定目标速度，例如
        //      * motor_set_target_speed(MOTOR_A, (float)(frame[2] | (frame[3] << 8)));
        //      * motor_set_target_speed(MOTOR_B, (float)(frame[4] | (frame[5] << 8)));
        //      */
        //     UART_Send_Buf(PRINT_INST, frame, UART_RX_FRAME_SIZE);   /* 回显，测试用 */
        // }




        /* ---------- 每 200ms 刷新一次显示 + 串口上报 ---------- */
        if (++ui_tick >= UI_PERIOD_MS) {
            uint32_t spd_a  = (uint32_t)motor_a.current_speed;
            uint32_t spd_b  = (uint32_t)motor_b.current_speed;
            uint32_t duty_a = motor_a.duty;
            uint32_t duty_b = motor_b.duty;
            uint32_t tgt    = (uint32_t)motor_a.target_speed;

            ui_tick = 0U;

            /* 灰度采样 + 串口发送（不再用 500ms 阻塞延时，避免拖慢显示/控制） */
            huidu_get_value();
            uart_send_huidu();

            /* 一行一个轮子：速度 + PWM，两轮对照看 */
            OLED_ShowString(0, 0, (u8 *)"A S:", 16);
            OLED_ShowNum(32, 0, spd_a, 4, 16);
            OLED_ShowString(64, 0, (u8 *)"P:", 16);
            OLED_ShowNum(80, 0, duty_a, 4, 16);

            OLED_ShowString(0, 16, (u8 *)"B S:", 16);
            OLED_ShowNum(32, 16, spd_b, 4, 16);
            OLED_ShowString(64, 16, (u8 *)"P:", 16);
            OLED_ShowNum(80, 16, duty_b, 4, 16);

            OLED_ShowString(0, 32, (u8 *)"TGT:", 16);
            OLED_ShowNum(32, 32, tgt, 4, 16);
            OLED_ShowString(64, 32, (u8 *)"mm/s", 16);

            OLED_ShowString(0, 48, (u8 *)((running != 0U) ? "RUN " : "STOP"), 16);
            OLED_Refresh();

            while(DMP_Read_Data(&pitch, &roll, &yaw));

            UART_Send_String(PRINT_INST, "A SPD=");
            uart_send_u32(spd_a);
            UART_Send_String(PRINT_INST, " PWM=");
            uart_send_u32(duty_a);
            UART_Send_String(PRINT_INST, " | B SPD=");
            uart_send_u32(spd_b);
            UART_Send_String(PRINT_INST, " PWM=");
            uart_send_u32(duty_b);
            UART_Send_String(PRINT_INST, "\r\n");
        }

        delay_ms(1);
    }
}
