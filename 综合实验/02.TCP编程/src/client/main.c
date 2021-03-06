#include <stdio.h>
#include <string.h>
#include "dmac.h"
#include "unistd.h"
#include "fpioa.h"
#include "gpiohs.h"
#include "uarths.h"
#include "bsp.h"
#include "spi.h"
#include "sysctl.h"
#include "pin_config.h"
#include "plic.h"
#include "sleep.h"
#include "esp32_spi_io.h"
#include "esp32_spi.h"
#include "sleep.h"
#include "uart.h"

// #define TEST_NETWORK

char* WIFI_SSID = "DaGuLion";
char* WIFI_PASSWD = "abc360abc";

uint8_t ip[] = {192, 168, 1, 130};
uint16_t port = 8096;

void hardware_init(void)
{
    /* ESP32 */
    fpioa_set_function(PIN_ESP_CS,   FUNC_ESP_SPI_CS);
    fpioa_set_function(PIN_ESP_RST,  FUNC_ESP_SPI_RST);
    fpioa_set_function(PIN_ESP_RDY,  FUNC_ESP_SPI_RDY);
    fpioa_set_function(PIN_ESP_MOSI, FUNC_ESP_SPI_MOSI);
    fpioa_set_function(PIN_ESP_MISO, FUNC_ESP_SPI_MISO);
    fpioa_set_function(PIN_ESP_CLK,  FUNC_ESP_SPI_CLK);

    /* 使能SPI0和DVP */
    sysctl_set_spi0_dvp_data(1);
}

uint8_t connect_server_by_ip_port(uint8_t* host, uint16_t port)
{
    uint8_t sock = esp32_spi_get_socket();

    if (sock != 0xff)
    {
        if (esp32_spi_socket_connect(sock, ip, 0, port, TCP_MODE) != 0)
        {
            return 0xff;
        }
    }
    else
    {
        return 0xff;
    }
    return sock;
}

int main(void)
{
    /* 硬件引脚初始化 */
    hardware_init();

    /* 调试串口初始化 */
    uarths_init();
    esp32_spi_config_io(FUNC_ESP_SPI_CS - FUNC_GPIOHS0, FUNC_ESP_SPI_RST, FUNC_ESP_SPI_RDY - FUNC_GPIOHS0, FUNC_ESP_SPI_MOSI - FUNC_GPIOHS0, FUNC_ESP_SPI_MISO - FUNC_GPIOHS0, FUNC_ESP_SPI_CLK - FUNC_GPIOHS0);

    esp32_spi_init();
    
    while (esp32_spi_connect_AP(WIFI_SSID, WIFI_PASSWD, 5) != 0);

    printf("wifi connection state: %d....\n", esp32_spi_is_connected());

#ifdef TEST_NETWORK
    printf("------------------------------------------\n");

    int ping_www_count = 5;
    int ping_www_success = 0;
    while(ping_www_count--)
    {
        ping_www_success = esp32_spi_ping("www.baidu.com", 1, 255);
        printf("ping www.baidu.com: %d ms\n", ping_www_success);
        if(ping_www_success > 0)
            break;
        sleep(1);
    }

    printf("------------------------------------------\n");

    int ping_ip_count = 5;
    int ping_ip_success = 0;
    uint8_t ip_array[] = {192, 168, 1, 130};
    while(ping_ip_count--)
    {
        ping_ip_success = esp32_spi_ping(ip_array, 0, 255);
        printf("ping 192.168.1.130: %d ms\n", ping_ip_success);
        if(ping_ip_success > 0 && ping_ip_success < 255)
            break;
        sleep(1);
    }

    printf("------------------------------------------\n");
#endif
    uint8_t sock = connect_server_by_ip_port(ip, port);
    if(sock == 0xff)
    {
        printf("connect server error !\n");
        return -1;
    }
    char* str = "Hello World !";
    int str_len = strlen(*str);
    uint8_t* uint8_str = (uint8_t *)malloc(str_len);
    memcpy(uint8_str, str, str_len);
    int count = 5;
    while (1)
    {
        int writen_len = esp32_spi_socket_write(sock, (uint8_t *)str, 13);
        if(!writen_len)
        {
            printf("wirte socket err !\n");
            return -2;
        }
        if(0 == count--)
        {
            esp32_spi_socket_close(sock);
            break;
        }
        sleep(2);
    }
    free(uint8_str);
    while(1);

    return 0;
}