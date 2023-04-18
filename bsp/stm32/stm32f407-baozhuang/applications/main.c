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
#include <dfs_posix.h> /* ����Ҫʹ���ļ�����ʱ����Ҫ�������ͷ�ļ� */

/* defined the LED0 pin: PG1 */
#define LED0_PIN    GET_PIN(G, 1)

int voltage_set = 500,current_set = 1500; //�洢�ĵ�ѹ��������ֵ��(Ĭ�ϵ�ѹΪ5V������Ϊ100��������Ϊ15A������Ϊ100��)
void para_read(){   //��W25Q64�ж�ȡ����
  int fd;
	uint8_t params[5] = {0};  
	fd = open("/params.dat", O_WRONLY | O_CREAT);
	if(fd >= 0){	//��ȡ�ɹ�
			read(fd, params, sizeof(params));
			close(fd);	
			if(params[0] > 0 || params[1] > 0 || params[2] > 0 || params[3] > 0){  //����Ѿ��������
				voltage_set = (params[0]<<8) + params[1];
				current_set = (params[2]<<8) + params[3];
			}
			else{      //���δ�������
				para_write();
			}
	}
	else{  //��ȡʧ�ܣ��ٴ�1�β���
				char *type = "elm";
				int result = 0;
				result = dfs_mkfs(type, "W25Q64");
				if (result != RT_EOK)
				{
						rt_kprintf("mkfs failed, result=%d\n", result);
				}
				para_write();
	}
}

void para_write(void){   //��W25Q64��д������
  int fd;
	uint8_t params[4] = {0};
	params[0] = (((uint16_t)voltage_set) & 0xff00)>>8; //��ѹ��8λ
	params[1] = (((uint16_t)voltage_set) & 0x00ff);    //��ѹ��8λ
	params[2] = (((uint16_t)current_set) & 0xff00)>>8;
	params[3] = (((uint16_t)current_set) & 0x00ff);
	fd = open("/params.dat", O_WRONLY);
	if(fd >= 0){	//��ȡ�ɹ�
			write(fd, params, sizeof(params));
			close(fd);	
	}	
}

int main(void)
{
    int count = 1;
		para_read();  //�ϵ��ȡflash�еĲ���
		read_knob_vol_thread();
		extern void MX_TIM1_Init(void);
		MX_TIM1_Init();  ///��ʱ��1PWMģʽ��ʼ��
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

int run_record = 0; //������־λ
int pwm1_status[4] = {0};  //����ǰpwmͨ���Ŀ���״̬��1����ͨ��0����ر�,pwm1_status[1]����ͨ��1
extern int pwm1_start; 
extern struct rt_semaphore pwm1_sem;
extern int percentage_change; //�ٷֱ��л���־λ�����ñ�־λΪ1������ͨ��1��percentage�����ı�

void PHASE_U_cb(){  //�жϻص�����
	if((run_record == 1 && pwm1_status[1] == 0) || percentage_change == 3){
		percentage_change--;
		pwm1_start = 1;  //�����ͨ��1 
		rt_sem_release(&pwm1_sem);
	}
}

void PHASE_V_cb(){
	if((run_record == 1 && pwm1_status[2] == 0) || percentage_change == 2){
		percentage_change--;
		pwm1_start = 2;  //�����ͨ��2 
		rt_sem_release(&pwm1_sem);
	}	
}

void PHASE_W_cb(){
	if((run_record == 1 && pwm1_status[3] == 0) || percentage_change == 1){
		percentage_change--;
		pwm1_start = 3;  //�����ͨ��3
		rt_sem_release(&pwm1_sem);
	}	
}

void PHASE_U_isr(void *args)   //U���ű仯�ᴥ�����жϴ�������
{
	static int32_t last_tick = 0;
	int32_t tick = rt_tick_get();
		if(tick - last_tick > 3 || tick - last_tick < -3){   //����������������ã��޳�����3��tick����Ӧ��
			PHASE_U_cb();                               
		}
		last_tick = tick;
}

void PHASE_V_isr(void *args)   //V���ű仯�ᴥ�����жϴ�������
{
	static int32_t last_tick = 0;
	int32_t tick = rt_tick_get();
		if(tick - last_tick > 3 || tick - last_tick < -3){   //����������������ã��޳�����3��tick����Ӧ��
			PHASE_V_cb();                               
		}
		last_tick = tick;
}

void PHASE_W_isr(void *args)   //W���ű仯�ᴥ�����жϴ�������
{
	static int32_t last_tick = 0;
	int32_t tick = rt_tick_get();
		if(tick - last_tick > 3 || tick - last_tick < -3){   //����������������ã��޳�����3��tick����Ӧ��
			PHASE_W_cb();                               
		}
		last_tick = tick;
}

int pin_setting(){
		//������������Ϊ��������
		rt_pin_mode(GPIO_PHASE_U, PIN_MODE_INPUT_PULLUP);
		rt_pin_mode(GPIO_PHASE_V, PIN_MODE_INPUT_PULLUP);
		rt_pin_mode(GPIO_PHASE_W, PIN_MODE_INPUT_PULLUP);
		rt_pin_mode(GPIO_MI1, PIN_MODE_INPUT_PULLUP);
		rt_pin_mode(GPIO_MI2, PIN_MODE_INPUT_PULLUP);	
		//��������Ϊ���ģʽ
		rt_pin_mode(GPIO_MO1, PIN_MODE_OUTPUT);
			
		//����U,V,WΪ�жϴ�(�������½��ض������ķ�ʽ)
		rt_pin_attach_irq(GPIO_PHASE_U, PIN_IRQ_MODE_RISING_FALLING, PHASE_U_isr, 0); 
		rt_pin_attach_irq(GPIO_PHASE_V, PIN_IRQ_MODE_RISING_FALLING, PHASE_V_isr, 0); 
		rt_pin_attach_irq(GPIO_PHASE_W, PIN_IRQ_MODE_RISING_FALLING, PHASE_W_isr, 0); 
    rt_pin_irq_enable(GPIO_PHASE_U,1);   //ʹ��3���ж�
    rt_pin_irq_enable(GPIO_PHASE_V,1);
    rt_pin_irq_enable(GPIO_PHASE_W,1);
	
		rt_pin_write(GPIO_MO1, 1);
		return 0;
}

GPIO_Port gpio_port;
static GPIO_Port port_pre, port_in;
//static GPIO_Port port_last;
#define ALARM_MIN_UPLOAD_TIME 1
#define ALARM_MAX_UPLOAD_TIME 1000

void read_all_pin(GPIO_Port * port)   //��ȡ�������ŵĵ�ƽ��ͬʱ��bit0~bit19���Ӧ���������Ű󶨣��Կ�����ɫ�����Ӧʵ�����Ÿߵ�ƽ��
{
    port->input.bit1 = !rt_pin_read(GPIO_PHASE_U);  //�͵�ƽ�����⵽��ֵΪ1
		port->input.bit2 = !rt_pin_read(GPIO_PHASE_V);
		port->input.bit3 = !rt_pin_read(GPIO_PHASE_W);
		port->input.bit4 = !rt_pin_read(GPIO_MI1);
		port->input.bit5 = !rt_pin_read(GPIO_MI2);
}

void gpio_entry(void *parameters)
{
		pin_setting();  //������ص������������
    rt_bool_t stabled = 0;
    rt_tick_t sensor_tick = rt_tick_get();  //��ȡ��ǰ��tickʱ��	
		for (;;)   //һֱ����forѭ��֮�У�ʱ�̻�ȡ�����������ŵĵ�λ
		{
				if(gpio_port.input.bit4 == 0 && gpio_port.input.bit5 == 1)  //��������
						run_record = 1;
        do
        {
						port_pre.input.word = port_in.input.word;
            read_all_pin(&port_in);  //�����е��������ŵĵ�λ��ȡ��port_in
            stabled = (port_pre.input.word == port_in.input.word) ;  //��ȡ�������ŵ��ȶ���ֵ
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
//        port_last = gpio_port;
				rt_thread_mdelay(1);				
		}
}

static int gpio_thread_init(void)  //��ʼ��gpio
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

int count = 0;
rt_err_t control_hwtimer_cb(rt_device_t dev, rt_size_t size) //��ʱ����callback������Ҫ�ǽ���״̬��״̬ת������Ӧ(�����е��÷�������ĺ���)
{
		count++;
    return 0;
}

static int timer14_init(void){  //��ʼ����ʱ��14
	rt_err_t ret = RT_EOK;
	rt_hwtimerval_t timeout_s; /* ��ʱ����ʱֵ */
	rt_device_t hw_dev = RT_NULL; /* ��ʱ���豸��� */
	rt_hwtimer_mode_t mode; /* ��ʱ��ģʽ */
	hw_dev = rt_device_find("timer14");
	RT_ASSERT(hw_dev != RT_NULL)
	ret = rt_device_open(hw_dev, RT_DEVICE_OFLAG_RDWR);
	RT_ASSERT(ret == RT_EOK)
	rt_device_set_rx_indicate(hw_dev, control_hwtimer_cb);
	mode = HWTIMER_MODE_PERIOD;
	ret = rt_device_control(hw_dev, HWTIMER_CTRL_MODE_SET, &mode);
	RT_ASSERT(ret == RT_EOK);
	timeout_s.sec = 0;
	timeout_s.usec = 100;  //0.1ms����1���жϻص�����
	if (rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s))
			!= sizeof(timeout_s)) {
		RT_ASSERT(0);
	}
	return 0;
}
INIT_APP_EXPORT(timer14_init);
