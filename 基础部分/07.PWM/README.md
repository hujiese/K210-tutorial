## PWM

PWM 控制的是脉冲输出的占空比，占空比是指在一个脉冲循环内，通电时间相对于总时间所占的比例。比如说，一个 RGB 灯在它一个工作周期中有一半时间
被接通了，那么它的占空比就是 50%，同时亮度也只有 50%。如果加在该工作元件上的信号电压为 3V，则实际的工作电压平均值或电压有效值就是 1.5V。

上一章提到，210 芯片定时器总共有 3 个，每个定时器有 4 路通道。每个定时器可以设置触发间隔，和定时器中断处理函数。定时器还可以设置为 PWM 输出的功能，但是如果设置 PWM 输出，则不可以使用定时的功能。

本次实验将通过一个呼吸灯的案例来介绍K210 的PWM使用。实验的原理是，通过PWM输出控制RGB三色灯中的红灯的电压，从而控制红灯的亮度，由于需要不断调整PWM的脉宽，所以这里还使用到了一个定时器。

实验代码详见src/pwm。

首先是IO口配置：

```c
#ifndef _PIN_CONFIG_H_
#define _PIN_CONFIG_H_
/*****************************HEAR-FILE************************************/
#include "fpioa.h"

/*****************************HARDWARE-PIN*********************************/
// 硬件IO口，与原理图对应
#define PIN_RGB_R             (6)

#endif /* _PIN_CONFIG_H_ */
```

接下来初始化硬件和全局中断：

```c
/* 硬件引脚初始化 */
hardware_init();

/* 系统中断初始化和使能 */
plic_init();
sysctl_enable_irq();
```

hardware_init代码如下：

```c
void hardware_init(void)
{
    fpioa_set_function(PIN_RGB_R, FUNC_TIMER2_TOGGLE2);
}
```

这里和前面不同，前面的点亮RGB或者按键案例都是绑定到FUNC_GPIOHS0等输入输出IO功能上，如果想让该物理IO口具有PWM输出能力，则应该绑定到FUNC_TIMER2_TOGGLE2这类功能上，这类功能有：

```c
	FUNC_TIMER0_TOGGLE1   = 190,    /*!< TIMER0 Toggle Output 1 */
    FUNC_TIMER0_TOGGLE2   = 191,    /*!< TIMER0 Toggle Output 2 */
    FUNC_TIMER0_TOGGLE3   = 192,    /*!< TIMER0 Toggle Output 3 */
    FUNC_TIMER0_TOGGLE4   = 193,    /*!< TIMER0 Toggle Output 4 */
    FUNC_TIMER1_TOGGLE1   = 194,    /*!< TIMER1 Toggle Output 1 */
    FUNC_TIMER1_TOGGLE2   = 195,    /*!< TIMER1 Toggle Output 2 */
    FUNC_TIMER1_TOGGLE3   = 196,    /*!< TIMER1 Toggle Output 3 */
    FUNC_TIMER1_TOGGLE4   = 197,    /*!< TIMER1 Toggle Output 4 */
    FUNC_TIMER2_TOGGLE1   = 198,    /*!< TIMER2 Toggle Output 1 */
    FUNC_TIMER2_TOGGLE2   = 199,    /*!< TIMER2 Toggle Output 2 */
    FUNC_TIMER2_TOGGLE3   = 200,    /*!< TIMER2 Toggle Output 3 */
    FUNC_TIMER2_TOGGLE4   = 201,    /*!< TIMER2 Toggle Output 4 */
```

K210的三个定时器分别有四路通道，每路通道都可以作用PWM输出，但是如果该定时器用作PWM功能，则不可用于普通定时器功能，PWM和定时功能只能使用一个，不可同时使用。这里选择的是定时器2的第二路通道。

接下来初始化PWM：

```c
void init_pwm(void)
{
    /* 初始化PWM */
    pwm_init(PWM_DEVICE_2);
    /* 设置PWM频率为200KHZ，占空比为0.5的方波 */
    pwm_set_frequency(PWM_DEVICE_2, PWM_CHANNEL_1, 200000, 0.5);
    /* 使能 PWM 输出 */
    pwm_set_enable(PWM_DEVICE_2, PWM_CHANNEL_1, 1);
}
```

首先调用pwm_init初始化一个PWM设备，由于前面选择定时器2作为PWM输出，所以这里设置为PWM_DEVICE_2，可以根据所使用的定时器来选择不同的PWM设备：

```c
typedef enum _pwm_device_number
{
    PWM_DEVICE_0,
    PWM_DEVICE_1,
    PWM_DEVICE_2,
    PWM_DEVICE_MAX,
} pwm_device_number_t;
```

接下来调用pwm_set_frequency设置输出PWM的脉宽和频率，该函数定义如下：

```c
double pwm_set_frequency(pwm_device_number_t pwm_number, pwm_channel_number_t channel, double frequency, double duty)
```

* 参数1为PWM的设备号

* 参数2为PWM所用的通道号，这个通道号有：

  ```c
  typedef enum _pwm_channel_number
  {
      PWM_CHANNEL_0,
      PWM_CHANNEL_1,
      PWM_CHANNEL_2,
      PWM_CHANNEL_3,
      PWM_CHANNEL_MAX,
  } pwm_channel_number_t;
  ```

  通道号的选择和之前的IO口所设置的管脚对应，以定时器2为例:

  ```c
  FUNC_TIMER2_TOGGLE1  ->  PWM_CHANNEL_0
  FUNC_TIMER2_TOGGLE2  ->  PWM_CHANNEL_1
  FUNC_TIMER2_TOGGLE3  ->  PWM_CHANNEL_2
  FUNC_TIMER2_TOGGLE4  ->  PWM_CHANNEL_3
  ```

  总结就是：

  ```c
  FUNC_TIMERx_TOGGLEy -> PWM_DEVICE_(y-1)
  FUNC_TIMERx_ -> PWM_DEVICE_x
  ```

* 参数3是PWM的频率，这里设置为200000Hz，频率越慢，呼吸灯的效果越明显
* 参数4为PWM的占空比，初始设置为0.5，占空比为50%

最后调用pwm_set_enable使能PWM，该函数定义如下：

```c
void timer_set_enable(timer_device_number_t timer_number, timer_channel_number_t channel, uint32_t enable)
```

* 参数1为PWM设备号
* 参数2为通道号
* 参数3用于使能PWM，1为使能，0为关闭

有了这些之后再设置一个定时器，该定时器用来定时改变PWM的占空比，该定时器设置如下：

```c
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
```

定时器没10ms定时调用timer_timeout_cb，改变PWM的占空比，timer_timeout_cb函数定义如下：

```c
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
```

该函数让PWM的占空比在0-1之间不断变动，每次变化1%。

通过如下命令编译烧写代码到开发板：

```c
cmake .. -DPROJ=pwm -G "MinGW Makefiles"
make
kflash .\pwm.bin -p COM6 -b 1500000
```

可以观察到，红色LED灯的亮度在不断变化。