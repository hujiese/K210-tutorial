## 定时器

定时器非常好理解，假设你现在办公，但是定了个闹钟5点停下吃饭，到了五点闹钟响了，你停下办公区去吃饭了，吃完放继续回来办公。

接下来将通过一个案例来说明K210的定时器的用法，详细代码可见src/timer。

案例的需求是，定时0.5m让RGB灯中的R灯亮，通过独立按键的按下/松开来控制定时器的开关。

硬件IO相关的配置如下所示：、

```c
#ifndef _PIN_CONFIG_H_
#define _PIN_CONFIG_H_
/*****************************HEAR-FILE************************************/
#include "fpioa.h"

/*****************************HARDWARE-PIN*********************************/
// 硬件IO口，与原理图对应
#define PIN_RGB_R             (6)

#define PIN_KEY               (16)

/*****************************SOFTWARE-GPIO********************************/
// 软件GPIO口，与程序对应
#define RGB_R_GPIONUM          (0)

#define KEY_GPIONUM            (3)

/*****************************FUNC-GPIO************************************/
// GPIO口的功能，绑定到硬件IO口
#define FUNC_RGB_R             (FUNC_GPIOHS0 + RGB_R_GPIONUM)

#define FUNC_KEY               (FUNC_GPIOHS0 + KEY_GPIONUM)

#endif /* _PIN_CONFIG_H_ */
```

硬件初始化相关的代码比较固定，和前面的案例类似，这里直接进入正题。

定时器初始化代码如下：

```c
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
```

在上述代码中，使用timer_init来初始化一个定时器，这里设置的是定时器0，可选的定时器有：

```c
typedef enum _timer_deivce_number
{
    TIMER_DEVICE_0,
    TIMER_DEVICE_1,
    TIMER_DEVICE_2,
    TIMER_DEVICE_MAX,
} timer_device_number_t;
```

然后调用timer_set_interval函数设置定时器的定时时间，该函数原型如下：

```c
size_t timer_set_interval(timer_device_number_t timer_number, timer_channel_number_t channel, size_t nanoseconds)
```

* 参数1为定时器号
* 参数2为定时器通道，每路定时器可设置五个通道：

 ```c
 typedef enum _timer_channel_number
 {
     TIMER_CHANNEL_0,
     TIMER_CHANNEL_1,
     TIMER_CHANNEL_2,
     TIMER_CHANNEL_3,
     TIMER_CHANNEL_MAX,
 } timer_channel_number_t;
 ```

* 参数3为定时时间，单位为ns

紧接着调用timer_irq_register函数注册定时器回调函数，该函数定义如下：

```c
int timer_irq_register(timer_device_number_t device, timer_channel_number_t channel, int is_single_shot, uint32_t priority, timer_callback_t callback, void *ctx)
```

* 参数1为定时器号
* 参数2为定时器通道号
* 参数3用于设置定时方式，设置为1则表示只定时一次，设置为0表示循环定时触发，例如，本次实验设置为0，表示没过0.5s定时触发一次
* 参数4为定时器中断优先级
* 参数5为定时器中断回调函数
* 参数6为定时器回调函数传入参数

最后通过timer_set_enable函数使能定时器，该函数定义如下：

```c
void timer_set_enable(timer_device_number_t timer_number, timer_channel_number_t channel, uint32_t enable)
```

* 参数1为定时器号

* 参数2为定时器的通道号

* 参数3用于使能/关闭定时器

本次案例中的按键采用了中断的方式，按键中断配置代码如下：

```c
void init_key(void)
{
    // 设置按键的GPIO模式为上拉输入
    gpiohs_set_drive_mode(KEY_GPIONUM, GPIO_DM_INPUT_PULL_UP);
    // 设置按键的GPIO电平触发模式为上升沿和下降沿
    gpiohs_set_pin_edge(KEY_GPIONUM, GPIO_PE_BOTH);
    // 设置按键GPIO口的中断回调
    gpiohs_irq_register(KEY_GPIONUM, 1, key_irq_cb, NULL);

}
```

中断回调函数key_irq_cb定义如下：

```c
int key_irq_cb(void* ctx)
{
    gpio_pin_value_t key_state = gpiohs_get_pin(KEY_GPIONUM);

    if (key_state)
        timer_set_enable(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1);
    else
        timer_set_enable(TIMER_DEVICE_0, TIMER_CHANNEL_0, 0);
    return 0;
}
```

在该回调函数中读取按键的状态，如果按下则关闭定时器，否则使能定时器。

该案例的主程序如下：

```c
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
```

在build目录使用如下命令编译烧写代码到开发板：

```
cmake .. -DPROJ=timer -G "MinGW Makefiles"
make
kflash .\timer.bin -p COM6 -b 1500000
```

开发板现象是每隔0.5m亮灭一次红灯，按下独立按键，红灯不在处于亮灭循环状态，松开按键，红灯每隔0.5s亮灭一次。