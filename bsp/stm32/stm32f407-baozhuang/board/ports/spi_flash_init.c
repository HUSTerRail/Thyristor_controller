/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-27     SummerGift   add spi flash port file
 */

#include <rtthread.h>
#include "spi_flash.h"
#include "spi_flash_sfud.h"
#include "drv_spi.h"

#if defined(BSP_USING_SPI_FLASH)
static int rt_hw_spi_flash_init(void)
{
    __HAL_RCC_GPIOG_CLK_ENABLE();
    rt_hw_spi_device_attach("spi2", "spi20", GPIOG, GPIO_PIN_12);

    if (RT_NULL == rt_sfud_flash_probe("W25Q64", "spi20"))
    {
        LOG_E("spi falsh probe error");
        return -RT_ERROR;
    };

    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_spi_flash_init);


#endif


#ifdef RT_USING_DFS
#include <dfs_fs.h>
#include <dfs_posix.h> /* 当需要使用文件操作时，需要包含这个头文件 */
int mnt_init(void)
{
	//rt_thread_delay(RT_TICK_PER_SECOND);
#if 0
	int res = dfs_mkfs("elm",  "W25Q64");
	if( 0 != res)
	{
		rt_kprintf("mkfs failed!\n");
	}
#endif
	if (dfs_mount("W25Q64", "/", "elm", 0, 0) == 0) {
		log_i("W25Q64 mount successful!\n");
	} else {

		log_e("W25Q64 mount failed!\n");
	}
	return 0;
}
INIT_APP_EXPORT(mnt_init);


static void readwrite_sample(void)
{
    int fd, size;
    char s[] = "RT-Thread Programmer!", buffer[80];

    log_i("Write string %s to test.txt.\n", s);

    /* 以创建和读写模式打开 /text.txt 文件，如果该文件不存在则创建该文件 */
    fd = open("/text.txt", O_WRONLY | O_CREAT);
    if (fd>= 0)
    {
        write(fd, s, sizeof(s));
        close(fd);
        log_i("Write done.\n");
    }

      /* 以只读模式打开 /text.txt 文件 */
    fd = open("/text.txt", O_RDONLY);
    if (fd>= 0)
    {
        size = read(fd, buffer, sizeof(buffer));
        close(fd);
        log_i("Read from file test.txt : %s \n", buffer);
        if (size < 0)
            return ;
    }
 }
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(readwrite_sample, readwrite sample);
#endif

