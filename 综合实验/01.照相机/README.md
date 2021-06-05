## 照相机

这是个较为综合的实验，实验涉及到前面的相当多内容：

* 独立按键中断
* LCD显示
* DVP摄像头
* 读写SD卡

新涉及到的内容有图片压缩。这里有必要提到图片压缩，之前摄像头拍摄到的图片在LCD上显示是RGB565格式的，可以将图片保存为RGB565格式，但这里推荐使用另一种保存方式--jpeg格式。RGB565格式图片体积较大，而jpeg图片可通过RGB565图片压缩得到，所以体积更小，但是图片经过压缩后分辨率或有所下降。

实验的代码在src/camera_cap目录下，其中camera_cap/hardware目录为硬件的驱动代码，camera_cap/utils目录下为图像的压缩编码库。

首先是硬件配置和初始化：

```c
/* 硬件引脚初始化 */
hardware_init();

/* 设置IO口电压 */
io_set_power();

/* 设置系统时钟和DVP时钟 */
sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
sysctl_pll_set_freq(SYSCTL_PLL1, 300000000UL);
sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
uarths_init();

/* 系统中断初始化，使能全局中断*/
plic_init();
sysctl_enable_irq();

/* 初始化LCD */
lcd_init();

// 初始化按键key
init_key();

/* 初始化摄像头 */
ov2640_init();

/* 设置拍照flag为0 */
g_save_flag = 0;

g_ram_mux = 0;

if (check_sdcard())
{
    printf("SD card err\n");
    return -1;
}

if (check_fat32())
{
    printf("FAT32 err\n");
    return -1;
}
```

这部分的代码在之前的章节中全部都介绍过，而且相对固定，这里不再过多描述。

接下来看业务逻辑方面的代码：

```c
while (1)
{
    /* 等待摄像头采集结束，然后清除结束标志 */
    while (g_dvp_finish_flag == 0)
        ;
    g_ram_mux ^= 0x01;
    g_dvp_finish_flag = 0;

    if (g_save_flag)
    {
        // 根据按键次数生成.jpeg格式文件
        char filename[16];
        sprintf(filename, "0:%08d.jpeg", g_count++);
        // 保存图片到sd卡中
        if(!save_jpeg_sdcard((uint8_t*)(g_ram_mux ? display_buf_addr1 : display_buf_addr2), filename, CONFIG_JPEG_COMPRESS_QUALITY))
        {
            --g_count;
            printf("save img fail !\n");
        }
        g_save_flag = 0;
    }

    /* 显示画面 */
    lcd_draw_picture(0, 0, 320, 240, g_ram_mux ? display_buf_addr1 : display_buf_addr2);
}
iomem_free(display_buf_addr1);
iomem_free(display_buf_addr2);
```

这段代码中，首先等待独立按键按下：

```c
/* 等待摄像头采集结束，然后清除结束标志 */
while (g_dvp_finish_flag == 0)
    ;
```

独立按键的中断回调函数定义如下：

```c
int key_irq_cb(void* ctx)
{
    g_save_flag = 1;

    return 0;
}
```

按下按键之后，会调用sprintf函数，根据按键按下的次数来生成一个文件名，然后调用save_jpeg_sdcard函数将图片保存到SD卡中。

需要注意的是，这里的图片拍摄和显示采取了双缓冲的策略，拍摄好的图片放到display_buf_addr1地址供其他代码调用，然后下一次拍摄的图片放到display_buf_addr2中，这样依次循环。

save_jpeg_sdcard函数定义如下：

```c
int save_jpeg_sdcard(uint8_t *image_addr, const char *filename, int img_quality)
{
    FIL file;
    FRESULT ret = FR_OK;
    uint32_t ret_len = 0;

    if (convert_image2jpeg(image_addr, img_quality) == 0)
    {

        if ((ret = f_open(&file, filename, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK)
        {
            printf("create file %s err[%d]", filename, ret);
            return ret;
        }

        f_write(&file, jpeg_out.data, jpeg_out.bpp, &ret_len);

        f_close(&file);
        printf("Save jpeg image %s\n", filename);
        return 1;
    }
    else
    {
        return 0;
    }
}
```

在该函数中，首先将拍摄图片转化为jpeg格式，convert_image2jpeg函数定义如下，传入参数为rgb565图像字节数组和Quality 转换质量：

```c
static int convert_image2jpeg(uint8_t *image, int Quality)
{
    uint64_t v_tim;
    v_tim = sysctl_get_time_us();

    jpeg_src.w = CONFIG_CAMERA_RESOLUTION_WIDTH;
    jpeg_src.h = CONFIG_CAMERA_RESOLUTION_HEIGHT;
    jpeg_src.bpp = 2;
    jpeg_src.data = image;

    jpeg_out.w = jpeg_src.w;
    jpeg_out.h = jpeg_src.h;
    jpeg_out.bpp = CONFIG_JPEG_BUF_LEN * 1024;
    jpeg_out.data = jpeg_buf;

    v_tim = sysctl_get_time_us();
    uint8_t ret = jpeg_compress(&jpeg_src, &jpeg_out, Quality, 0);
    if (ret == 0)
    {
        printf("jpeg encode use %ld us\n", sysctl_get_time_us() - v_tim);
        printf("w:%d\th:%d\tbpp:%d\n", jpeg_out.w, jpeg_out.h, jpeg_out.bpp);
        printf("jpeg encode success!\n");
    }
    else
    {
        printf("jpeg encode failed\n");
    }

    return ret;
}
```

回到save_jpeg_sdcard函数，将拍摄图片转为jpeg格式之后便在SD卡中创建一个图片文件，将该图片内容保存到该图片文件中。

到这里，拍照程序便完成了。