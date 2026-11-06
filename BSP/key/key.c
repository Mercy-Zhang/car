#include "key.h"
#include "motor.h"
/**
 * 中断处理函数
 */
//volatile uint8_t key0_pressed = 0U;

/**
 * @brief 按键初始化：清标志 + 开 NVIC
 * @note  外设端（GPIO 边沿、上拉）由 SYSCFG_DL_GPIO_init() 使能
 */
// void KEY_Init(void)
// {
//     /* 先清一次，防止上电时挂起的标志立刻进中断 */
//     DL_GPIO_clearInterruptStatus(KEY_PORT, KEY_KEY0_PIN);
//     NVIC_ClearPendingIRQ(KEY_INT_IRQN);
//     NVIC_EnableIRQ(KEY_INT_IRQN);
// }

/**
 * @brief GROUP1 中断：GPIOA / GPIOB 共用同一个向量（GPIOA_INT_IRQn == GPIOB_INT_IRQn == 1）
 *
 * 必须按 group 分发，并且每个 group 都要把自己的标志清掉。
 * 以前只处理 GPIOA，按键(GPIOB)一按标志永远清不掉 -> 中断风暴，主循环被饿死。
 * 现在 GPIOA 处理 A 轮编码器 E1A，GPIOB 处理 B 轮编码器 E2A；
 * 按键 KEY0 也在 GPIOB 上，暂时不处理（读到 IIDX 时标志已被清掉，不会中断风暴）。
 */
void GROUP1_IRQHandler(void)
{
    switch (DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1))
    {
    case MOTOR_GPIOA_INT_IIDX:                        /* GPIOA：A 轮编码器 E1A */
        if (DL_GPIO_getPendingInterrupt(MOTOR_E1A_PORT) == MOTOR_E1A_IIDX) {
            DL_GPIO_clearInterruptStatus(MOTOR_E1A_PORT, MOTOR_E1A_PIN);
            motor_encoder_on_edge(MOTOR_A);
        }
        break;

    case GPIO_MULTIPLE_GPIOB_INT_IIDX:                /* GPIOB：B 轮编码器 E2A（KEY0 暂不处理）*/
        if (DL_GPIO_getPendingInterrupt(MOTOR_E2A_PORT) == MOTOR_E2A_IIDX) {
            DL_GPIO_clearInterruptStatus(MOTOR_E2A_PORT, MOTOR_E2A_PIN);
            motor_encoder_on_edge(MOTOR_B);
        }
        break;

    // case KEY_INT_IIDX:                          /* GPIOB：KEY0 */
    //     if (DL_GPIO_getPendingInterrupt(KEY_PORT) == KEY_KEY0_IIDX) {
    //         DL_GPIO_clearInterruptStatus(KEY_PORT, KEY_KEY0_PIN);
    //         key0_pressed = 1U;                  /* 只置标志，消抖放主循环 */
    //     }
    //     break;

    default:
        break;
    }
}
