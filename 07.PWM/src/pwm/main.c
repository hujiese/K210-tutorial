#include <stdio.h>
#include <unistd.h>
#include "gpio.h"
#include "timer.h"
#include "pwm.h"
#include "sysctl.h"
#include "plic.h"
#include "pin_config.h"

void hardware_init(void)
{

    fpioa_set_function(PIN_RGB_R, FUNC_TIMER2_TOGGLE2);
}

int timer_timeout_cb(void *ctx) {
    static double duty_cycle = 0.01;
    /* 0为渐增，1为渐减 */
    static int flag = 0;      

    /* 传入cycle的不同值，调节PWM的占用比，也就是调节灯的亮度 */
    pwm_set_frequency(PWM_DEVICE_2, PWM_CHANNEL_1, 200000, duty_cycle);

    /* 修改cycle的值，让其在区间（0,1）内渐增和渐减 */
    flag ? (duty_cycle -= 0.01): (duty_cycle += 0.01);
    if(duty_cycle > 1.0)
    {
        duty_cycle = 1.0;
        flag = 1;
    }
    else if (duty_cycle < 0.0)
    {
        duty_cycle = 0.0;
        flag = 0;
    }
    return 0;
}

void init_timer(void) {
    /* 定时器初始化 */
    timer_init(TIMER_DEVICE_0);
    /* 设置定时器超时时间，单位为ns */
    timer_set_interval(TIMER_DEVICE_0, TIMER_CHANNEL_0, 10 * 1e6);
    /* 设置定时器中断回调 */
    timer_irq_register(TIMER_DEVICE_0, TIMER_CHANNEL_0, 0, 1, timer_timeout_cb, NULL);
    /* 使能定时器 */
    timer_set_enable(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1);
}

// FUNC_TIMER1_TOGGLE1  ->  PWM_CHANNEL_0
// FUNC_TIMER1_TOGGLE2  ->  PWM_CHANNEL_1
// FUNC_TIMER1_TOGGLE3  ->  PWM_CHANNEL_2
// FUNC_TIMER1_TOGGLE4  ->  PWM_CHANNEL_3

// FUNC_TIMERx_ -> PWM_DEVICE_x

void init_pwm(void)
{
    /* 初始化PWM */
    pwm_init(PWM_DEVICE_2);
    /* 设置PWM频率为200KHZ，占空比为0.5的方波 */
    pwm_set_frequency(PWM_DEVICE_2, PWM_CHANNEL_1, 200000, 0.5);
    /* 使能 PWM 输出 */
    pwm_set_enable(PWM_DEVICE_2, PWM_CHANNEL_1, 1);
}

int main(void)
{
    /* 硬件引脚初始化 */
    hardware_init();

    /* 系统中断初始化和使能 */
    plic_init();
    sysctl_enable_irq();
    
    /* 初始化定时器 */
    init_timer();

    /* 初始化PWM */
    init_pwm();

    while(1);

    return 0;
}
