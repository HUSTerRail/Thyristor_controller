/*
 * 程序清单： DAC 设备使用例程
 * 例程导出了 dac_sample 命令到控制终端
 * 命令调用格式：dac_sample
 * 程序功能：通过 DAC 设备将数字值转换为模拟量，并输出电压值。
 *           示例代码参考电压为3.3V,转换位数为12位。
*/
//通道1：代表AO1;通道2：代表AO2
#include <rtthread.h>
#include <rtdevice.h>
#include <stdlib.h>
#include "stdint.h"
#define DAC_DEV_NAME        "dac1"  /* DAC 设备名称 */
#define DAC_DEV_CHANNEL1     1       /* DAC 通道1 */
#define DAC_DEV_CHANNEL2     2       /* DAC 通道2 */

#define REFER_VOLTAGE       1006         /* 参考电压 3.3V,数据精度乘以100保留2位小数*/
#define CONVERT_BITS        (1 << 12)   /* 转换位数为12位 */

void dac1(int dac_value)
{
    rt_dac_device_t dac_dev;
    rt_uint32_t value, vol;

    /* 查找设备 */
    dac_dev = (rt_dac_device_t)rt_device_find(DAC_DEV_NAME);
    if (dac_dev == RT_NULL)
    {
        rt_kprintf("dac sample run failed! can't find %s device!\n", DAC_DEV_NAME);
    }

    /* 打开通道 */
    rt_dac_enable(dac_dev, DAC_DEV_CHANNEL1);

    /* 设置输出值 */
    value = dac_value;
    rt_dac_write(dac_dev, DAC_DEV_CHANNEL1, value);
    rt_kprintf("the value is :%d \n", value);

    /* 转换为对应电压值 */
    vol = value * REFER_VOLTAGE / CONVERT_BITS;
    rt_kprintf("the voltage is :%d.%02d \n", vol / 100, vol % 100);
}
void dac2(int dac_value)
{
    rt_dac_device_t dac_dev;
    rt_uint32_t value, vol;

    /* 查找设备 */
    dac_dev = (rt_dac_device_t)rt_device_find(DAC_DEV_NAME);
    if (dac_dev == RT_NULL)
    {
        rt_kprintf("dac sample run failed! can't find %s device!\n", DAC_DEV_NAME);
    }

    /* 打开通道 */
    rt_dac_enable(dac_dev, DAC_DEV_CHANNEL2);

    /* 设置输出值 */
    value = dac_value;
    rt_dac_write(dac_dev, DAC_DEV_CHANNEL2, value);
    rt_kprintf("the value is :%d \n", value);

    /* 转换为对应电压值 */
    vol = value * REFER_VOLTAGE / CONVERT_BITS;
    rt_kprintf("the voltage is :%d.%02d \n", vol / 100, vol % 100);
}
