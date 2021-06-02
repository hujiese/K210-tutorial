#include <stdio.h>
#include "fpioa.h"
#include <string.h>
#include "uart.h"
#include "gpiohs.h"
#include "sysctl.h"
#include "pin_config.h"

int on_uart_recv(void *ctx)
{
    char v_buf[12];
    int ret =  uart_receive_data(UART_NUM, v_buf, 12);
    char *v_send_ok = "Send ok Uart: ";
    // printf("receive %s\n", v_buf);
    uart_send_data(UART_NUM, v_send_ok,strlen(v_send_ok));
    uart_send_data(UART_NUM, v_buf, ret);
    return 0;
}

void hardware_init(void)
{
    fpioa_set_function(PIN_UART_USB_RX, FUNC_UART_USB_RX);
    fpioa_set_function(PIN_UART_USB_TX, FUNC_UART_USB_TX);
}

int main()
{
    hardware_init();
    plic_init();
    sysctl_enable_irq();

    uart_init(UART_NUM);
    uart_configure(UART_NUM, 115200, 8, UART_STOP_1, UART_PARITY_NONE);

    uart_set_receive_trigger(UART_NUM, UART_RECEIVE_FIFO_8);
    uart_irq_register(UART_NUM, UART_RECEIVE, on_uart_recv, NULL, 2);

    char *msg = {"hello world!\n"};
    uart_send_data(UART_NUM, msg, strlen(msg));

    while(1);
}

