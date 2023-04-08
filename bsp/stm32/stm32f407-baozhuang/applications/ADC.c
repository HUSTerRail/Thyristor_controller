#include <rtthread.h>
#include "rtdevice.h"
#include <ulog.h>

//AI1:ADC1_IN0、AI2:ADC1_IN8
#define ADC_DEV_NAME        "adc1"      /* ADC 设备名称 AS1+*/
#define ADC_DEV_CHANNEL1     0           /* ADC 通道1 */
#define ADC_DEV_CHANNEL2     8           /* ADC 通道2 */
#define REFER_VOLTAGE       100         /* 参考电压 10V,数据精度乘以10保留1位小数*/
#define CONVERT_BITS        (1 << 12)   /* 转换位数为12位 */

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5
static rt_thread_t tid1 = RT_NULL;
int16_t adc_voltage1 = 0;
int16_t adc_voltage2 = 0;

/* 获取电压的范围：0V~21V ，单位0.1V*/
static void read_knob_vol_entry(void *parameter)
{
    rt_adc_device_t adc_dev;
    rt_uint32_t value1,value2;
//    static uint32_t last_vol1 = 0;
//    static uint32_t last_vol2 = 0;
//    float filter_factor = 0.1;  //滤波系数
    /* 查找设备 */
    adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
    if (adc_dev == RT_NULL)
    {
        log_e("adc sample run failed! can't find %s device!\n", ADC_DEV_NAME);
    }
    /* 使能设备 */
    rt_adc_enable(adc_dev, ADC_DEV_CHANNEL1);
    rt_adc_enable(adc_dev, ADC_DEV_CHANNEL2);
    while (1)
    {
        /* 读取采样值 */
        value1 = rt_adc_read(adc_dev, ADC_DEV_CHANNEL1);
        value2 = rt_adc_read(adc_dev, ADC_DEV_CHANNEL2);
        /* 转换为对应电压值 */
        adc_voltage1 = value1 * REFER_VOLTAGE / CONVERT_BITS;
        adc_voltage2 = value2 * REFER_VOLTAGE / CONVERT_BITS;
//        /* 将得到的电压值进行一阶低通滤波，去除毛刺和噪声 */
//        adc_voltage1 = (int32_t)(( adc_voltage1 * filter_factor ) + ( 1 - filter_factor ) * last_vol1); //计算
//        adc_voltage2 = (int32_t)(( adc_voltage2 * filter_factor ) + ( 1 - filter_factor ) * last_vol2); //计算
//        last_vol1 = adc_voltage1;                                     //存贮本次数据
//        last_vol2 = adc_voltage2;                                     //存贮本次数据
        rt_thread_mdelay(10);  //线程里面需要有延时函数
    }
}

int read_knob_vol_thread(void)
{
    tid1 = rt_thread_create("thread1",
                            read_knob_vol_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);
    return 0;
}
