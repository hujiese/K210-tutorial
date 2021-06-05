#include "sleep.h"
#include "gpiohs.h"
#include "pin_config.h"

void hardware_init(void)
{
    /* fpioa映射 */
    fpioa_set_function(PIN_RGB_R, FUNC_RGB_R);
    fpioa_set_function(PIN_RGB_G, FUNC_RGB_G);
    fpioa_set_function(PIN_RGB_B, FUNC_RGB_B);

    fpioa_set_function(PIN_KEYPAD_LEFT,   FUNC_KEYPAD_LEFT);
    fpioa_set_function(PIN_KEYPAD_MIDDLE, FUNC_KEYPAD_MIDDLE);
    fpioa_set_function(PIN_KEYPAD_RIGHT,  FUNC_KEYPAD_RIGHT);

}

void rgb_all_off(void)
{
    gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_HIGH);
    gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_HIGH);
    gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_HIGH);
}

void init_rgb(void)
{
    /* 设置RGB灯的GPIO模式为输出 */
    gpiohs_set_drive_mode(RGB_R_GPIONUM, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(RGB_G_GPIONUM, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(RGB_B_GPIONUM, GPIO_DM_OUTPUT);

    /* 关闭RGB灯 */
    rgb_all_off();
}

void init_keypad(void)
{
    /* 设置keypad的GPIO模式为上拉输入 */
    gpiohs_set_drive_mode(KEYPAD_LEFT_GPIONUM,   GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_drive_mode(KEYPAD_MIDDLE_GPIONUM, GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_drive_mode(KEYPAD_RIGHT_GPIONUM,  GPIO_DM_INPUT_PULL_UP);
}

void scan_keypad(void)
{
    /* 读取keypad三个通道的状态 */
    gpio_pin_value_t state_keypad_left =   gpiohs_get_pin(KEYPAD_LEFT_GPIONUM);
    gpio_pin_value_t state_keypad_middle = gpiohs_get_pin(KEYPAD_MIDDLE_GPIONUM);
    gpio_pin_value_t state_keypad_right =  gpiohs_get_pin(KEYPAD_RIGHT_GPIONUM);

    /* 检测keypad是否向左滚动 */
    if (!state_keypad_left)
    {
        /* 延迟消抖10ms */
        msleep(10);
        /* 再次读keypad向左的IO口的状态 */
        state_keypad_left = gpiohs_get_pin(KEYPAD_LEFT_GPIONUM);
        if (!state_keypad_left)
        {
            /* 向左滚动，点亮红灯 */
            gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_LOW);
        }
        else
        {
            /* 松开，红灯熄灭 */
            gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_HIGH);
        }
    } 
    /* 检测keypad是否被按下 */
    else if (!state_keypad_middle)
    {
        msleep(10);
        state_keypad_middle = gpiohs_get_pin(KEYPAD_MIDDLE_GPIONUM);
        if (!state_keypad_middle)
        {
            gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_LOW);
        }
        else
        {
            gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_HIGH);
        }
    } 
    /* 检测keypad是否向右滚动 */
    else if (!state_keypad_right)
    {
        msleep(10);
        state_keypad_right = gpiohs_get_pin(KEYPAD_RIGHT_GPIONUM);
        if (!state_keypad_right)
        {
            gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_LOW);
        }
        else
        {
            gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_HIGH);
        }
    }
}

int main(void)
{
    /* 硬件引脚初始化 */
    hardware_init();
    
    /* 初始化RGB灯 */
    init_rgb();

    /* 初始化keypad */
    init_keypad();

    while (1)
    {
        /* 扫描keypad并控制RGB灯 */
        scan_keypad();
    }

    return 0;
}
