#include "huidu.h"
#include "motor.h"
uint8_t huidu_value[] = {0,0,0,0,0};

uint8_t get_gpio_state(GPIO_Regs *gpio_port, uint32_t gpio)
{
    uint32_t high_bits = DL_GPIO_readPins(gpio_port,gpio);
    if((high_bits & gpio) != 0) return 1;
    else return 0;
}

void huidu_get_value(void)
{
    huidu_value[0] = get_gpio_state(HUIDU_L2_PORT, HUIDU_L2_PIN);
    huidu_value[1] = get_gpio_state(HUIDU_L1_PORT, HUIDU_L1_PIN);
    huidu_value[2] = get_gpio_state(HUIDU_M_PORT, HUIDU_M_PIN);
    huidu_value[3] = get_gpio_state(HUIDU_R1_PORT, HUIDU_R1_PIN);
    huidu_value[4] = get_gpio_state(HUIDU_R2_PORT, HUIDU_R2_PIN);
}

void adjust_motor()
{
    huidu_get_value();
    if(huidu_value[0] == 0 && huidu_value[1] == 0 && huidu_value[2] == 0 && huidu_value[3] == 0 && huidu_value[4] == 0)
    {   // 继续直走
        motor_set_direction(1,1);
        motor_set_direction(2,1);
        motor_set_target_speed(1,200);
        motor_set_target_speed(2,200);
    }
    else if(huidu_value[0] == 1 && huidu_value[1] == 1 && huidu_value[2] == 1 && huidu_value[3] == 1 && huidu_value[4] == 1)
    {
        //不对
        motor_stop();
    }
    else if(huidu_value[0] == 0 && huidu_value[1] == 0 && huidu_value[2] == 1 && huidu_value[3] == 0 && huidu_value[4] == 0)
    {
        // 继续直走
        motor_set_direction(1,1);
        motor_set_direction(2,1);
        motor_set_target_speed(1,200);
        motor_set_target_speed(2,200);
    }
    else if(huidu_value[0] == 0 && huidu_value[1] == 1)
    {
        //左偏
        motor_set_target_speed(1,400);
        motor_set_target_speed(2,300);
    }
    else if(huidu_value[0] == 1 && huidu_value[1] == 1)
    {
        //左偏很厉害
        motor_set_target_speed(1,500);
        motor_set_target_speed(2,200);
    }
    else if (huidu_value[0] == 1)
    {
        motor_set_target_speed(1,500);
        motor_set_target_speed(2,500);
    }
}