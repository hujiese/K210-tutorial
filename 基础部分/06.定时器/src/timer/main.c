#include "pin_config.h"
#include "sleep.h"
#include "gpiohs.h"
#include "sysctl.h"
#include "plic.h"
#include "timer.h"

uint32_t g_count;

void hardware_init(void)
{
    /* fpioa映射 */
    fpioa_set_function(PIN_RGB_R, FUNC_RGB_R);

    fpioa_set_function(PIN_KEY, FUNC_KEY);
}

void init_rgb(void)
{
    /* 设置RGB灯的GPIO模式为输出 */
    gpiohs_set_drive_mode(RGB_R_GPIONUM, GPIO_DM_OUTPUT);

    /* 关闭RGB灯 */
    gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_HIGH);
}

int key_irq_cb(void* ctx)
{
    gpio_pin_value_t key_state = gpiohs_get_pin(KEY_GPIONUM);

    if (key_state)
        timer_set_enable(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1);
    else
        timer_set_enable(TIMER_DEVICE_0, TIMER_CHANNEL_0, 0);
    return 0;
}

void init_key(void)
{
    // 设置按键的GPIO模式为上拉输入
    gpiohs_set_drive_mode(KEY_GPIONUM, GPIO_DM_INPUT_PULL_UP);
    // 设置按键的GPIO电平触发模式为上升沿和下降沿
    gpiohs_set_pin_edge(KEY_GPIONUM, GPIO_PE_BOTH);
    // 设置按键GPIO口的中断回调
    gpiohs_irq_register(KEY_GPIONUM, 1, key_irq_cb, NULL);

}

int timer_timeout_cb(void *ctx) {
    uint32_t *tmp = (uint32_t *)(ctx);
    (*tmp)++;
    if ((*tmp)%2)
    {
       gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_LOW);
    }
    else
    {
        gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_HIGH);
    }
    return 0;
}

void init_timer(void) {
    /* 定时器初始化 */
    timer_init(TIMER_DEVICE_0);
    /* 设置定时器超时时间，单位为ns */
    timer_set_interval(TIMER_DEVICE_0, TIMER_CHANNEL_0, 500 * 1e6);
    /* 设置定时器中断回调 */
    timer_irq_register(TIMER_DEVICE_0, TIMER_CHANNEL_0, 0, 1, timer_timeout_cb, &g_count);
    /* 使能定时器 */
    timer_set_enable(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1);
}

int main(void)
{
    /* 硬件引脚初始化 */
    hardware_init();

    /* 初始化系统中断并使能 */
    plic_init();
    sysctl_enable_irq();

    /* 初始化RGB灯 */
    init_rgb();

    /* 初始化按键key */
    init_key();

    /* 初始化定时器 */
    init_timer();
    
    while (1);

    return 0;
}
