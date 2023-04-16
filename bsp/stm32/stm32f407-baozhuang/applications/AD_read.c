#include <rtthread.h>
#include "rtdevice.h"
#include <ulog.h>

//AI1:ADC1_IN0、AI2:ADC1_IN8
#define ADC_DEV_NAME        "adc1"      /* ADC 设备名称 AS1+*/
#define REFER_VOLTAGE       1000         /* 参考电压10V,数据精度乘以100保留2位小数*/
#define CONVERT_BITS        (1 << 12)   /* 转换位数为12位 */

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5
static rt_thread_t tid1 = RT_NULL;
int adc_voltage[7] = {0};
int last_voltage[7] = {0};

/* 获取电压的范围：0V~21V ，单位0.1V*/
static void read_knob_vol_entry(void *parameter)
{
    rt_adc_device_t adc_dev;
    float filter_factor = 0.1;  //滤波系数
    /* 查找设备 */
    adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
    if (adc_dev == RT_NULL)
    {
        log_e("adc sample run failed! can't find %s device!\n", ADC_DEV_NAME);
    }
    /* 使能设备 */
		for(int i = 0;i < 7;i++){  //使能ADC1通道0~6
			rt_adc_enable(adc_dev, i);
		}
		extern int voltage_set;
		extern int pwm1_end;
		extern int pwm1_status[4];
		extern struct rt_semaphore pwm1_sem;
    while (1)
    {
        /* rt_adc_read(adc_dev, i)读取采样值 */
        /* 并转换为对应电压值 */
				for(int i = 0;i < 7;i++){
					adc_voltage[i] = rt_adc_read(adc_dev, i) * REFER_VOLTAGE / CONVERT_BITS;
				}
        /* 将得到的电压值进行一阶低通滤波，去除毛刺和噪声 */
				for(int i = 0;i < 7;i++){
        adc_voltage[i] = (int32_t)(( adc_voltage[i] * filter_factor ) + ( 1 - filter_factor ) * last_voltage[i]); //计算
				last_voltage[i] = adc_voltage[i];                                     //存贮本次数据
				}
//				for(int i = 2;i <= 4;i++){  //为U、V、W电压，检测是否到达设定电压值。
//						if(adc_voltage[i] >= voltage_set && pwm1_status[i-1]){  //达到电压值且处于PWM通道处于打开状态
//								pwm1_end = i - 1;  //代表关闭通道i-1
//								rt_sem_release(&pwm1_sem);
//						}
//				}
        rt_thread_mdelay(5);  //线程里面需要有延时函数
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
