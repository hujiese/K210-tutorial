/**
* @par  Copyright (C): 2016-2022, SCUT
* @file         main.c
* @author       jackster
* @version      V1.0
* @date         2020.10.19
* @brief        摄像头显示当前画面，拍照存入SD卡中
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: 摄像头采集当前画面，然后在LCD上显示;通过boot按键拍照将图片保存为jpeg格式存入sd卡根目录中。
*/
#include <stdio.h>
#include <string.h>
#include "fpioa.h"
#include "gpiohs.h"
#include "uarths.h"
#include "bsp.h"
#include "sysctl.h"
#include "plic.h"
#include "sleep.h"
#include "iomem.h"
#include "htonx.h"
#include "pin_config.h"
#include "net_utils.h"

// WIFI热点和密码
char* WIFI_SSID = "DaGuLion";
char* WIFI_PASSWD = "abc360abc";
// 用于调试联网
#define TEST_NETWORK

#define MAX_IMG_LEN (1024 * 32)
#define NET_IMG_BUF (MAX_IMG_LEN + 4)

void esp32_config(void)
{
    /* ESP32 */
    fpioa_set_function(PIN_ESP_CS,   FUNC_ESP_SPI_CS);
    fpioa_set_function(PIN_ESP_RST,  FUNC_ESP_SPI_RST);
    fpioa_set_function(PIN_ESP_RDY,  FUNC_ESP_SPI_RDY);
    fpioa_set_function(PIN_ESP_MOSI, FUNC_ESP_SPI_MOSI);
    fpioa_set_function(PIN_ESP_MISO, FUNC_ESP_SPI_MISO);
    fpioa_set_function(PIN_ESP_CLK,  FUNC_ESP_SPI_CLK);
}

void hardware_init(void)
{
    esp32_config();
    sysctl_set_spi0_dvp_data(1);

    esp32_spi_config_io(FUNC_ESP_SPI_CS - FUNC_GPIOHS0, FUNC_ESP_SPI_RST, FUNC_ESP_SPI_RDY - FUNC_GPIOHS0, FUNC_ESP_SPI_MOSI - FUNC_GPIOHS0, FUNC_ESP_SPI_MISO - FUNC_GPIOHS0, FUNC_ESP_SPI_CLK - FUNC_GPIOHS0);

    /* 初始化esp32 */
    esp32_spi_init();
}

int main(void)
{
    hardware_init();    
    
    while (esp32_spi_connect_AP(WIFI_SSID, WIFI_PASSWD, 5) != 0);

    printf("wifi connection state: %d....\n", esp32_spi_is_connected());

#ifdef TEST_NETWORK
    test_network();
#endif

	uint16_t port = 8080;
	uint8_t sock = 1;
    allocate_socket(sock);
	int8_t started = esp32_spi_start_server(sock, 0, 0, port, TCP_MODE);
    if (!started)
	{
		printf("Server started on port %d\n", port);
	}
	else
	{
		printf("Server failed to start\n");
	}
    while(1)
    {
        int client_sock = esp32_spi_socket_available(sock);
        if (client_sock != 255 && client_sock != -1)
        {
            printk("New client connect %d\n", client_sock);
            allocate_socket(client_sock);

        }
        while(client_connected(&client_sock))
        {
            if(available(client_sock))
            {
                memset(msg, 0, MSGLEN);
                client_read(client_sock, msg, MSGLEN);
                printk("read from client: ");
                for(int i = 0; i < 8; ++i)
                {
                    printk("%c", msg[i]);
                }
            }
        }
        client_stop(client_sock);
    }

    return 0;
}