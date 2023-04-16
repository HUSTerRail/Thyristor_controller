/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 * 2018-11-19     flybreak     add stm32f407-atk-explorer bsp
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <definitions.h>
#include <dfs_fs.h>
#include <dfs_posix.h> /* 当需要使用文件操作时，需要包含这个头文件 */

/* defined the LED0 pin: PG1 */
#define LED0_PIN    GET_PIN(G, 1)

int voltage_set = 0,current_set = 0,percentage_set = 0; //存储的电压、电流、百分比的值。
void para_read(){   //从W25Q64中读取数据
  int fd, size;
	uint8_t params[5] = {0};  
	fd = open("/params.dat", O_WRONLY | O_CREAT);
	if(fd >= 0){	//读取成功
			size = read(fd, params, sizeof(params));
			close(fd);	
			voltage_set = (params[0]<<8) + params[1];
			current_set = (params[2]<<8) + params[3];
			percentage_set = params[4];
	}
	else{  //读取失败
			voltage_set = 0;
			current_set = 0;
			percentage_set = 0;
	}
}

void para_write(){   //向W25Q64中写入数据
  int fd;
	uint8_t params[5] = {0};
//	params[0] = (((uint16_t)voltage_set) & 0xff00)>>8; //电压高8位
//	params[1] = (((uint16_t)voltage_set) & 0x00ff);    //电压低8位
//	params[2] = (((uint16_t)current_set) & 0xff00)>>8;
//	params[3] = (((uint16_t)current_set) & 0x00ff);
//	params[4] = (((uint16_t)percentage_set) & 0xff);
	params[0] = 0x11;
	fd = open("/params.dat", O_WRONLY);
	if(fd >= 0){	//读取成功
			write(fd, params, sizeof(params));
			close(fd);	
	}	
}

int main(void)
{
    int count = 1;
		para_read();
		para_write();
		read_knob_vol_thread();
		pwm_led_sample();
    /* set LED0 pin mode to output */
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    while (count++)
    {
        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }

    return RT_EOK;
}

int run_record = 0; //启动标志位
int pwm1_status[4] = {0};  //代表当前pwm通道的开关状态，1代表导通，0代表关闭,pwm1_status[1]代表通道1
extern int pwm1_start; 
extern struct rt_semaphore pwm1_sem;

void PHASE_U_cb(){  //中断回调函数
	if(run_record == 1 && pwm1_status[1] == 0){
		pwm1_start = 1;  //代表打开通道1 
		rt_sem_release(&pwm1_sem);
	}
}

void PHASE_V_cb(){
	if(run_record == 1 && pwm1_status[2] == 0){
		pwm1_start = 2;  //代表打开通道2 
		rt_sem_release(&pwm1_sem);
	}	
}

void PHASE_W_cb(){
	if(run_record == 1 && pwm1_status[3] == 0){
		pwm1_start = 3;  //代表打开通道3
		rt_sem_release(&pwm1_sem);
	}	
}

void PHASE_U_isr(void *args)   //U引脚变化会触发本中断处理函数。
{
	static int32_t last_tick = 0;
	int32_t tick = rt_tick_get();
		if(tick - last_tick > 3 || tick - last_tick < -3){   //进行消抖的相关配置，剔除大于3个tick的响应。
			PHASE_U_cb();                               
		}
		last_tick = tick;
}

void PHASE_V_isr(void *args)   //V引脚变化会触发本中断处理函数。
{
	static int32_t last_tick = 0;
	int32_t tick = rt_tick_get();
		if(tick - last_tick > 3 || tick - last_tick < -3){   //进行消抖的相关配置，剔除大于3个tick的响应。
			PHASE_V_cb();                               
		}
		last_tick = tick;
}

void PHASE_W_isr(void *args)   //W引脚变化会触发本中断处理函数。
{
	static int32_t last_tick = 0;
	int32_t tick = rt_tick_get();
		if(tick - last_tick > 3 || tick - last_tick < -3){   //进行消抖的相关配置，剔除大于3个tick的响应。
			PHASE_W_cb();                               
		}
		last_tick = tick;
}

int pin_setting(){
		//设置输入引脚为上拉输入
		rt_pin_mode(GPIO_PHASE_U, PIN_MODE_INPUT_PULLUP);
		rt_pin_mode(GPIO_PHASE_V, PIN_MODE_INPUT_PULLUP);
		rt_pin_mode(GPIO_PHASE_W, PIN_MODE_INPUT_PULLUP);
		rt_pin_mode(GPIO_MI1, PIN_MODE_INPUT_PULLUP);
		rt_pin_mode(GPIO_MI2, PIN_MODE_INPUT_PULLUP);	
		//设置引脚为输出模式
		rt_pin_mode(GPIO_MO1, PIN_MODE_OUTPUT);
			
		//设置U,V,W为中断触(上升沿下降沿都触发的方式)
		rt_pin_attach_irq(GPIO_PHASE_U, PIN_IRQ_MODE_RISING_FALLING, PHASE_U_isr, 0); 
		rt_pin_attach_irq(GPIO_PHASE_V, PIN_IRQ_MODE_RISING_FALLING, PHASE_V_isr, 0); 
		rt_pin_attach_irq(GPIO_PHASE_W, PIN_IRQ_MODE_RISING_FALLING, PHASE_W_isr, 0); 
	
		rt_pin_write(GPIO_MO1, 1);
		return 0;
}

GPIO_Port gpio_port;
static GPIO_Port port_pre, port_in,port_last;
static GPIO_Port port_last;
#define ALARM_MIN_UPLOAD_TIME 1
#define ALARM_MAX_UPLOAD_TIME 1000

void read_all_pin(GPIO_Port * port)   //读取输入引脚的电平，同时将bit0~bit19与对应的输入引脚绑定（显控屏红色代表对应实际引脚高电平）
{
    port->input.bit1 = !rt_pin_read(GPIO_PHASE_U);  //低电平代表检测到，值为1
		port->input.bit2 = !rt_pin_read(GPIO_PHASE_V);
		port->input.bit3 = !rt_pin_read(GPIO_PHASE_W);
		port->input.bit4 = !rt_pin_read(GPIO_MI1);
		port->input.bit5 = !rt_pin_read(GPIO_MI2);
}

void gpio_entry(void *parameters)
{
		pin_setting();  //引脚相关的输入输出配置
    rt_bool_t stabled = 0;
    rt_tick_t sensor_tick = rt_tick_get();  //获取当前的tick时间	
		for (;;)   //一直处于for循环之中，时刻获取所有输入引脚的电位
		{
				run_record = 1;  //测试专用
				if(gpio_port.input.bit4 == 0 && gpio_port.input.bit5 == 1)  //代表启动
						run_record = 1;
        do
        {
						port_pre.input.word = port_in.input.word;
            read_all_pin(&port_in);  //将所有的输入引脚的电位读取到port_in
            stabled = (port_pre.input.word == port_in.input.word) ;  //读取到了引脚的稳定数值
            if (!stabled)
            {
                rt_thread_mdelay(1);
            }

        }
        while (!stabled);

        if(gpio_port.input.word != port_in.input.word){
        	gpio_port.input.word = port_in.input.word;
        }

        if (rt_tick_get() - sensor_tick > ALARM_MAX_UPLOAD_TIME)
        {
            sensor_tick = rt_tick_get();
        }		
        port_last = gpio_port;
				rt_thread_mdelay(1);				
		}
}

static int gpio_thread_init(void)  //初始化gpio
{
    rt_thread_t tid = RT_NULL;

    tid = rt_thread_create("gpio", gpio_entry, RT_NULL, 1024, 15, 100); //10
    if (tid == RT_NULL)
    {
        return -RT_ENOMEM;
    }
    rt_thread_startup(tid);

    return RT_EOK;
}
INIT_APP_EXPORT(gpio_thread_init);
