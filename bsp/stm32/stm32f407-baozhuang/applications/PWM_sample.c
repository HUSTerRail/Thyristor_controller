/*
 * 程序清单：这是一个 PWM 设备使用例程
*/

#include <rtthread.h>
#include <rtdevice.h>
#include <definitions.h>
#include <board.h>

#define PWM_DEV_NAME        "pwm1"  /* PWM设备名称 */

struct rt_device_pwm *pwm_dev;      /* PWM设备句柄 */
/* 用于PWM的信号量 */
struct rt_semaphore pwm1_sem;
int pwm1_start = 0; //pwm1_start = 1代表要开启通道1
int percentage_set = 5;  //百分比设定，最开始按照百分之5去充电
rt_uint32_t period = 10000000;    /* 周期为10ms，单位为纳秒ns */
rt_uint32_t pulse = 1000000;     /* PWM脉冲宽度值1ms，单位为纳秒ns */
extern int count;//定时器计数值，定时器定时时间为0.1ms

extern int pwm1_status[4];
extern int run_record;
void pwm_thread_entry(void *parameter){
		while(1){		
				rt_sem_take(&pwm1_sem, RT_WAITING_FOREVER);
				if(pwm1_start != 0){   //代表要启动PWM通道
					/* 使能通道 */
					count = 0;
					while(count <= period/100000 *(100 - percentage_set)/100); //等待百分比延时时间后启动PWM通道
					rt_pwm_set(pwm_dev, pwm1_start, period*2, pulse*2);  //设置脉宽为pulse，开始发送脉冲
//					pwm1_status[pwm1_start] = 1;
					pwm1_start = 0;
				}
				else   //代表要关闭所有PWM通道
				{
					/* 关闭通道 */
					for(int i = 1; i < 4; i++){
						rt_pwm_set(pwm_dev, i, period*2, 0);  //设置脉宽为0，停止发送脉冲
						pwm1_status[i] = 0;
						percentage_set = 5;  //继续给百分比设置初值，为下一次充电做准备
						run_record  = 0;
						rt_pin_write(GPIO_MO1,1); //继电器输出
						rt_thread_mdelay(1000);   //适当延时1S
						rt_pin_write(GPIO_MO1,0); //继电器停止输出
					}
				}
		}
}

int pwm_led_sample(void)
{
		/* 初始化信号量 */
    rt_sem_init(&pwm1_sem, "pwm1_sem", 0, RT_IPC_FLAG_FIFO);

    /* 查找设备 */
    pwm_dev = (struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);
    if (pwm_dev == RT_NULL)
    {
        rt_kprintf("pwm sample run failed! can't find %s device!\n", PWM_DEV_NAME);
        return RT_ERROR;
    }
    /* 设置PWM周期和脉冲宽度默认值 */
		for(int i = 1;i < 4;i++)  //3路脉冲
		{
			rt_pwm_set(pwm_dev, i, period*2, 0);   //设置默认脉宽为0，为低电平
			rt_pwm_enable(pwm_dev, i); //使能PWM通道
		}

		rt_err_t ret = RT_EOK;
    /* 创建 PWM输出 线程 */
    rt_thread_t thread = rt_thread_create("serial", pwm_thread_entry, RT_NULL, 1024, 25, 10);
    /* 创建成功则启动线程 */
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        ret = RT_ERROR;
    }

    return ret;
}
