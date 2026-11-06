#ifndef MOTOR_H
#define MOTOR_H

#include "ti_msp_dl_config.h"

/* ---------- 电机编号（对应 TB6612 的 A / B 两路） ---------- */
#define MOTOR_A             1U          /* A 轮：A1N1/A1N2 + PWMA_C0 + E1A */
#define MOTOR_B             2U          /* B 轮：B1N1/B1N2 + PWMA_C1 + E2A */

/* ---------- 机械 / 编码器参数 ---------- */
#define MOTOR_PI            3.14159f
#define MOTOR_ENCODER_PPR   364U        /* 13 线 × 28 减速比 = 364 脉冲/圈（只数 E1A/E2A 上升沿）*/
#define MOTOR_WHEEL_D      65.0f       /* 轮径 mm */
#define MOTOR_CTRL_MS       10U         /* 控制周期 ms，必须和 syscfg 里 TIM_MOTOR 的 timerPeriod 一致 */
#define MOTOR_PWM_MAX       4000U       /* PWM 周期计数值，见 syscfg 的 PWM1.timerCount */

/* ---------- 方向定义（对应 TB6612 的 x1N1 / x1N2） ----------
 *  FWD  : N1=1 N2=0
 *  REV  : N1=0 N2=1
 *  BRAKE: N1=1 N2=1（短接制动）；若 N1=N2=0 则为滑行停止
 */
#define MOTOR_DIR_STOP     0
#define MOTOR_DIR_FWD       1
#define MOTOR_DIR_BACK      (-1)

/* ---------- 单个电机的运行状态（中断里更新，主循环只读） ----------
 *  A 轮用 motor_a，B 轮用 motor_b，用法完全对称。
 */
typedef struct {
    volatile float    current_speed;   /* 实测速度 mm/s */
    volatile float    target_speed;    /* 目标速度 mm/s */
    volatile uint16_t duty;            /* 当前占空比 0 ~ MOTOR_PWM_MAX */
} Motor_Status;

extern Motor_Status motor_a;
extern Motor_Status motor_b;

/* 初始化两轮：上电刹车、占空比 0、开编码器与控制环中断 */
void motor_init(void);

/* 设置占空比（直接写 PWM 比较值，供 PID / 调试使用） */
void motor_set_duty(uint8_t motor_id, uint16_t duty);

/* 设置方向：MOTOR_DIR_STOP / MOTOR_DIR_FWD / MOTOR_DIR_BACK */
void motor_set_direction(uint8_t motor_id, int8_t direction);

/* 设置单个轮子的目标速度（>0 时自动切前进方向），内部限幅 0 ~ 1000mm/s */
void motor_set_target_speed(uint8_t motor_id, float mm_s);

/* 两轮设成同一个目标速度 */
void motor_set_target_speed_both(float mm_s);

/* 两轮刹车并清掉 PID 内部状态 */
void motor_stop(void);

/* 编码器边沿回调，由 GROUP1_IRQHandler 调用（不要直接调用）*/
void motor_encoder_on_edge(uint8_t motor_id);

#endif // MOTOR_H
