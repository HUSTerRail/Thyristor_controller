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


/* defined the LED0 pin: PG1 */
#define LED0_PIN    GET_PIN(G, 1)


int main(void)
{
    int count = 1;
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

int pin_setting(){
		//������������Ϊ��������
		rt_pin_mode(GPIO_PHASE_U, PIN_MODE_INPUT_PULLUP);
		rt_pin_mode(GPIO_PHASE_V, PIN_MODE_INPUT_PULLUP);
		rt_pin_mode(GPIO_PHASE_W, PIN_MODE_INPUT_PULLUP);
		rt_pin_mode(GPIO_MI1, PIN_MODE_INPUT_PULLUP);
		rt_pin_mode(GPIO_MI2, PIN_MODE_INPUT_PULLUP);	
		//��������Ϊ���ģʽ
		rt_pin_mode(GPIO_MO1, PIN_MODE_OUTPUT);
		
		rt_pin_write(GPIO_MO1, 1);
		return 0;
}

GPIO_Port gpio_port;
static GPIO_Port port_pre, port_in,port_last;
static GPIO_Port port_last;
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
        port_last = gpio_port;
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
