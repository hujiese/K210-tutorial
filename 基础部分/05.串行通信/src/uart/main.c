#include <string.h>
#include "pin_config.h"


void hardware_init(void)
{
    // fpioa映射
    fpioa_set_function(PIN_UART_USB_RX, FUNC_UART_USB_RX);
    fpioa_set_function(PIN_UART_USB_TX, FUNC_UART_USB_TX);
}

int main(void)
{
    hardware_init();
    // 初始化串口3，设置波特率为115200
    uart_init(UART_USB_NUM);
    uart_configure(UART_USB_NUM, 115200, UART_BITWIDTH_8BIT, UART_STOP_1, UART_PARITY_NONE);

    /* Hello world! */
    char *hello = {"Hello world!\n"};
    uart_send_data(UART_USB_NUM, hello, strlen(hello));

    char recv = 0;

    while (1)
    {
        /* 等待串口信息，并通过串口发送出去 */
        while(uart_receive_data(UART_USB_NUM, &recv, 1))
        {
            uart_send_data(UART_USB_NUM, &recv, 1);
        }
    }
    return 0;
}
