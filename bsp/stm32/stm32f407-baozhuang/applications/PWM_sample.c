/*
 * 程序清单：这是一个 PWM 设备使用例程
*/

#include <rtthread.h>
#include <rtdevice.h>

#define PWM_DEV_NAME        "pwm1"  /* PWM设备名称 */

struct rt_device_pwm *pwm_dev;      /* PWM设备句柄 */
/* 用于PWM的信号量 */
struct rt_semaphore pwm1_sem;
int pwm1_start = 0,pwm1_end = 0; //pwm1_start = 1代表要开启通道1，pwm1_end = 1代表关闭通道1

extern int pwm1_status[4];
void pwm_thread_entry(void *parameter){
		while(1){
				rt_sem_take(&pwm1_sem, RT_WAITING_FOREVER);
				if(pwm1_start != 0){   //代表要启动PWM通道
					/* 使能通道 */
					rt_pwm_enable(pwm_dev, pwm1_start);	
					pwm1_status[pwm1_start] = 1;
					pwm1_start = 0;
				}
				else    //代表要关闭PWM通道
				{
					/* 关闭通道 */
					rt_pwm_disable(pwm_dev, pwm1_start);
					pwm1_status[pwm1_start] = 0;
					pwm1_end = 0;
				}
		}
}

int pwm_led_sample()
{
		/* 初始化信号量 */
    rt_sem_init(&pwm1_sem, "pwm1_sem", 0, RT_IPC_FLAG_FIFO);
	
    rt_uint32_t period, pulse;

    period = 500000;    /* 周期为0.5ms，单位为纳秒ns */
    pulse = 0;          /* PWM脉冲宽度值，单位为纳秒ns */

    /* 查找设备 */
    pwm_dev = (struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);
    if (pwm_dev == RT_NULL)
    {
        rt_kprintf("pwm sample run failed! can't find %s device!\n", PWM_DEV_NAME);
        return RT_ERROR;
    }
    /* 设置PWM周期和脉冲宽度默认值 */
		for(int i = 1;i < 4;i++)  //3路脉冲
    rt_pwm_set(pwm_dev, i, period, pulse);

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
