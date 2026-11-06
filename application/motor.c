#include "motor.h"
/**
 * 双轮驱动：A 轮 / B 轮各自独立计数、测速、PID。
 * 编码器中断累加脉冲，TIMA0 定时器 100Hz 里统一算速度、算 PWM。
 */
#define PID_KP   3.0f
#define PID_KI   0.2f

/* 每个轮子各自一份的 PID / 编码器内部状态（下标 0 = A 轮，1 = B 轮）*/
typedef struct {
    volatile uint32_t encode_count;     /* 编码器脉冲，在 GROUP1 中断里累加 */
    float             pwm_output;       /* 增量式 PID 的累加输出 */
    float             last_error;
} Motor_Priv;

static Motor_Priv motor_priv[2];

/* 对外暴露的运行状态，两轮完全对称 */
Motor_Status motor_a;
Motor_Status motor_b;

static Motor_Status *const motor_status_tab[2] = { &motor_a, &motor_b };

/* id 非法时返回 0，调用者先判断再取下标 */
static Motor_Status *motor_status(uint8_t motor_id)
{
    if ((motor_id != MOTOR_A) && (motor_id != MOTOR_B)) {
        return 0;
    }
    return motor_status_tab[motor_id - MOTOR_A];
}

/**
 * @brief 电机初始化：两轮上电刹车、占空比 0、开编码器与控制环中断
 */
void motor_init(void)
{
    Motor_Status *st;
    uint8_t       i;

    /* 上电先短接制动，占空比 0（PWM 定时器已由 SYSCFG_DL_init 启动）*/
    motor_set_direction(MOTOR_A, MOTOR_DIR_STOP);
    motor_set_direction(MOTOR_B, MOTOR_DIR_STOP);
    motor_set_duty(MOTOR_A, 0U);
    motor_set_duty(MOTOR_B, 0U);

    /* 清掉上电时可能残留的计数和 PID 状态 */
    for (i = 0U; i < 2U; i++) {
        motor_priv[i].encode_count = 0U;
        motor_priv[i].pwm_output   = 0.0f;
        motor_priv[i].last_error   = 0.0f;
    }
    st = motor_status(MOTOR_A);
    st->current_speed = 0.0f;
    st->target_speed  = 0.0f;
    st = motor_status(MOTOR_B);
    st->current_speed = 0.0f;
    st->target_speed  = 0.0f;

    /* 编码器计数走 GROUP1(GPIOA: E1A / GPIOB: E2A)，需要它比控制环更实时 */
    DL_GPIO_clearInterruptStatus(MOTOR_E1A_PORT, MOTOR_E1A_PIN);
    DL_GPIO_clearInterruptStatus(MOTOR_E2A_PORT, MOTOR_E2A_PIN);
    NVIC_ClearPendingIRQ(MOTOR_GPIOA_INT_IRQN);
    NVIC_ClearPendingIRQ(GPIO_MULTIPLE_GPIOB_INT_IRQN);
    NVIC_EnableIRQ(MOTOR_GPIOA_INT_IRQN);
    NVIC_EnableIRQ(GPIO_MULTIPLE_GPIOB_INT_IRQN);

    /* 控制环跑在 TIM_MOTOR(TIMA0) 的 100Hz LOAD 中断上，
       不要再挂到 PWM 定时器(TIMG0)上去 */
    DL_TimerA_clearInterruptStatus(TIM_MOTOR_INST, DL_TIMERA_INTERRUPT_LOAD_EVENT);
    NVIC_ClearPendingIRQ(TIM_MOTOR_INST_INT_IRQN);
    NVIC_SetPriority(TIM_MOTOR_INST_INT_IRQN, 1U);
    NVIC_EnableIRQ(TIM_MOTOR_INST_INT_IRQN);
}

/*-------------------------------------------目标转动方向---------------------------------------------------*/

/**
 * @brief 设置方向
 * @param motor_id  MOTOR_A / MOTOR_B
 * @param direction MOTOR_DIR_STOP / MOTOR_DIR_FWD / MOTOR_DIR_BACK
 */
void motor_set_direction(uint8_t motor_id, int8_t direction)
{
    GPIO_Regs *port;
    uint32_t   n1_pin;
    uint32_t   n2_pin;

    switch (motor_id) {
    case MOTOR_A:
        port   = MOTOR_A1N1_PORT;
        n1_pin = MOTOR_A1N1_PIN;
        n2_pin = MOTOR_A1N2_PIN;
        break;

    case MOTOR_B:
        port   = MOTOR_B1N1_PORT;
        n1_pin = MOTOR_B1N1_PIN;
        n2_pin = MOTOR_B1N2_PIN;
        break;

    default:
        return;
    }

    switch (direction) {
    case MOTOR_DIR_STOP:                       /* 双高：短接制动 */
        DL_GPIO_setPins(port, n1_pin | n2_pin);
        break;

    case MOTOR_DIR_FWD:
        DL_GPIO_setPins(port, n1_pin);
        DL_GPIO_clearPins(port, n2_pin);
        break;

    case MOTOR_DIR_BACK:
        DL_GPIO_clearPins(port, n1_pin);
        DL_GPIO_setPins(port, n2_pin);
        break;

    default:
        break;
    }
}

/**
 * @brief 设置单个轮子的目标速度（闭环），方向自动切到前进
 */
void motor_set_target_speed(uint8_t motor_id, float mm_s)
{
    Motor_Status *st = motor_status(motor_id);

    if (st == 0) {
        return;
    }

    if (mm_s < 0.0f) {
        mm_s = 0.0f;
    }
    if (mm_s > 1000.0f) {
        mm_s = 1000.0f;                         /* 保护：别把轮子甩飞 */
    }

    st->target_speed = mm_s;

    if (mm_s > 0.0f) {
        motor_set_direction(motor_id, MOTOR_DIR_FWD);
    } else {
        /* 目标 0：立刻刹车并清 PID 内部状态，避免下次启动有积分冲击 */
        motor_priv[motor_id - MOTOR_A].pwm_output = 0.0f;
        motor_priv[motor_id - MOTOR_A].last_error = 0.0f;
        motor_set_duty(motor_id, 0U);
        motor_set_direction(motor_id, MOTOR_DIR_STOP);
    }
}

/**
 * @brief 两轮设成同一个目标速度（直行）
 */
void motor_set_target_speed_both(float mm_s)
{
    motor_set_target_speed(MOTOR_A, mm_s);
    motor_set_target_speed(MOTOR_B, mm_s);
}


/*-----------------------------------计算当前速度-----------------------------------------------*/

/**
 * @brief 编码器 A 相上升沿回调（只计数，不做耗时动作）
 * @param motor_id MOTOR_A / MOTOR_B
 */
void motor_encoder_on_edge(uint8_t motor_id)
{
    if (motor_id == MOTOR_A) {
        motor_priv[0].encode_count++;
    } else if (motor_id == MOTOR_B) {
        motor_priv[1].encode_count++;
    }
}

/**
 * @brief 计算单个轮子的当前速度
 */
static void motor_calc_speed(uint8_t motor_id)
{
    Motor_Status *st  = motor_status(motor_id);
    uint8_t       idx = (uint8_t)(motor_id - MOTOR_A);
    uint32_t      count;
    uint32_t      primask = __get_PRIMASK();

    /* 读-清必须和编码器中断互斥，否则两次操作之间来的脉冲会被丢掉 */
    __disable_irq();
    count = motor_priv[idx].encode_count;
    motor_priv[idx].encode_count = 0U;
    __set_PRIMASK(primask);

    /* 10ms 内 count 个脉冲 -> mm/s */
    st->current_speed = (float)count / (float)MOTOR_ENCODER_PPR * MOTOR_PI * MOTOR_WHEEL_D * 100.0f;
}


//-----------------------------------------计算PWM输出----------------------------------------------//

/**
 * @brief 电机设置占空比
 * @param motor_id  MOTOR_A / MOTOR_B
 * @param duty      占空比 0 ~ MOTOR_PWM_MAX(4000)
 */
void motor_set_duty(uint8_t motor_id, uint16_t duty)
{
    Motor_Status *st = motor_status(motor_id);

    if (st == 0) {
        return;
    }
    if (duty > MOTOR_PWM_MAX) {
        duty = MOTOR_PWM_MAX;
    }

    if (motor_id == MOTOR_A) {
        DL_Timer_setCaptureCompareValue(PWMA_INST, duty, GPIO_PWMA_C0_IDX);
    } else {
        DL_Timer_setCaptureCompareValue(PWMA_INST, duty, GPIO_PWMA_C1_IDX);
    }
    st->duty = duty;
}

/**
 * @brief 增量式 PID：计算单个轮子的 PWM
 */
static void motor_control(uint8_t motor_id)
{
    Motor_Status *st  = motor_status(motor_id);
    uint8_t       idx = (uint8_t)(motor_id - MOTOR_A);
    float         error = st->target_speed - st->current_speed;
    float         delta = PID_KP * (error - motor_priv[idx].last_error) + PID_KI * error;
    float         pwm   = motor_priv[idx].pwm_output + delta;

    if (pwm > (float)MOTOR_PWM_MAX) {
        pwm = (float)MOTOR_PWM_MAX;
    }
    if (pwm < 0.0f) {
        pwm = 0.0f;
    }

    motor_priv[idx].pwm_output = pwm;
    motor_priv[idx].last_error = error;

    motor_set_duty(motor_id, (uint16_t)pwm);
}


/*-------------------------控制-----------------------------*/

/**
 * @brief 两轮刹车并清掉 PID 内部状态
 */
void motor_stop(void)
{
    motor_set_target_speed(MOTOR_A, 0.0f);
    motor_set_target_speed(MOTOR_B, 0.0f);
}


/**
 * @brief 控制环中断：TIMA0 每 10ms 一次（100Hz）
 */
void TIM_MOTOR_INST_IRQHandler(void)
{
    switch (DL_TimerA_getPendingInterrupt(TIM_MOTOR_INST))
    {
    case DL_TIMER_IIDX_LOAD:
        motor_calc_speed(MOTOR_A);
        motor_calc_speed(MOTOR_B);
        motor_control(MOTOR_A);
        motor_control(MOTOR_B);
        break;

    default:
        break;
    }
}
