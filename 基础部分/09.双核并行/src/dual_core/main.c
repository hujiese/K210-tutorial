#include <stdio.h>
#include "bsp.h"
#include "sleep.h"
#include "sysctl.h"

int core1_main(void *ctx)
{
    uint64_t core = current_coreid();
    printf("Core %ld say: Hello world\n", core);

    while(1)
    {
        sleep(1);
        printf("Core %ld is running too!\n", core);
    }
}

int main(void)
{
    /* 读取当前运行的核心编号 */
    uint64_t core = current_coreid();
    printf("Core %ld say: Hello world !\n", core);
    /* 注册核心1，并启动核心1 */
    register_core1(core1_main, NULL);

    while(1)
    {
        sleep(1);
        printf("Core %ld is running\n", core);
    }
    return 0;
}
