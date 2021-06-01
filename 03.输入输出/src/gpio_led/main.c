#include <stdio.h>
#include <unistd.h>
#include "gpio.h"
#include "pin_config.h"

void hardware_init(void)
{
    fpioa_set_function(PIN_LED_0, FUNC_LED0);
    fpioa_set_function(PIN_LED_1, FUNC_LED1);
}

int main(void)
{
    hardware_init();// 硬件引脚初始化

    gpio_init();    // 使能GPIO的时钟
    
    // 设置LED0和LED1的GPIO模式为输出
    gpio_set_drive_mode(LED0_GPIONUM, GPIO_DM_OUTPUT);
    gpio_set_drive_mode(LED1_GPIONUM, GPIO_DM_OUTPUT);
    
    // 先关闭LED0和LED1
    gpio_pin_value_t value = GPIO_PV_HIGH;
    gpio_set_pin(LED0_GPIONUM, value);
    gpio_set_pin(LED1_GPIONUM, value);

    while (1)
    {
        sleep(1);
        gpio_set_pin(LED0_GPIONUM, value);
        gpio_set_pin(LED1_GPIONUM, value = !value);
    }
    return 0;
}
