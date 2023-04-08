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

typedef struct _axis_port {
	int32_t encoder ;
	uint8_t enable;
	uint8_t alarm ;
} Axis_Port;


typedef struct _gpio_port {
	union {
		uint32_t word ;
		struct {
			uint32_t bit0 : 1;
			uint32_t bit1 : 1;
			uint32_t bit2 : 1;
			uint32_t bit3 : 1;
			uint32_t bit4 : 1;
			uint32_t bit5 : 1;
			uint32_t bit6 : 1;
			uint32_t bit7 : 1;
			uint32_t bit8 : 1;
			uint32_t bit9 : 1;
			uint32_t bit10 : 1;
			uint32_t bit11 : 1;
			uint32_t bit12 : 1;
			uint32_t bit13 : 1;
			uint32_t bit14 : 1;
			uint32_t bit15 : 1;
			uint32_t bit16 : 1;
			uint32_t bit17 : 1;
			uint32_t bit18 : 1;
			uint32_t bit19 : 1;
			uint32_t bit20 : 1;
			uint32_t bit21 : 1;
			uint32_t bit22 : 1;
			uint32_t bit23 : 1;
			uint32_t bit24 : 1;
			uint32_t bit25 : 1;
			uint32_t bit26 : 1;
			uint32_t bit27 : 1;
			uint32_t bit28 : 1;
			uint32_t bit29 : 1;
			uint32_t bit30 : 1;
			uint32_t bit31 : 1;
		};
	}input;
	union {
		int32_t word ;
		struct {
			uint32_t bit0 : 1;
			uint32_t bit1 : 1;
			uint32_t bit2 : 1;
			uint32_t bit3 : 1;
			uint32_t bit4 : 1;
			uint32_t bit5 : 1;
			uint32_t bit6 : 1;
			uint32_t bit7 : 1;
			uint32_t bit8 : 1;
			uint32_t bit9 : 1;
			uint32_t bit10 : 1;
			uint32_t bit11 : 1;
			uint32_t bit12 : 1;
			uint32_t bit13 : 1;
			uint32_t bit14 : 1;
			uint32_t bit15 : 1;
			uint32_t bit16 : 1;
			uint32_t bit17 : 1;
			uint32_t bit18 : 1;
			uint32_t bit19 : 1;
			uint32_t bit20 : 1;
			uint32_t bit21 : 1;
			uint32_t bit22 : 1;
			uint32_t bit23 : 1;
			uint32_t bit24 : 1;
			uint32_t bit25 : 1;
			uint32_t bit26 : 1;
			uint32_t bit27 : 1;
			uint32_t bit28 : 1;
			uint32_t bit29 : 1;
			uint32_t bit30 : 1;
			uint32_t bit31 : 1;
		};
	}output;

	Axis_Port axis[3];

} GPIO_Port;




/* defined the LED0 pin: PF9 */
#define LED0_PIN    GET_PIN(G, 10)

#define GPIO_MO1	GET_PIN(G, 0)
#define GPIO_MO2	GET_PIN(G, 1)
#define GPIO_MO3	GET_PIN(G, 2)
#define GPIO_MO4	GET_PIN(G, 3)
#define GPIO_MO5	GET_PIN(G, 4)
#define GPIO_MO6	GET_PIN(G, 5)
#define GPIO_MO7	GET_PIN(G, 6)
#define GPIO_MO8	GET_PIN(G, 7)
#define GPIO_MO9	GET_PIN(G, 8)
#define GPIO_MO10	GET_PIN(E, 0)
#define GPIO_MO11	GET_PIN(E, 1)
#define GPIO_MO12	GET_PIN(E, 2)
#define GPIO_MO13	GET_PIN(E, 3)
#define GPIO_MO14	GET_PIN(E, 4)
#define GPIO_MO15	GET_PIN(E, 6)
#define GPIO_MO16	GET_PIN(E, 7)

#define GPIO_MI1	GET_PIN(F, 0)
#define GPIO_MI2	GET_PIN(F, 1)
#define GPIO_MI3	GET_PIN(F, 2)
#define GPIO_MI4	GET_PIN(F, 3)
#define GPIO_MI5	GET_PIN(F, 4)
#define GPIO_MI6	GET_PIN(F, 5)
#define GPIO_MI7	GET_PIN(F, 6)
#define GPIO_MI8	GET_PIN(F, 7)
#define GPIO_MI9	GET_PIN(F, 8)
#define GPIO_MI10	GET_PIN(G, 15)
#define GPIO_MI11	GET_PIN(F, 10)
#define GPIO_MI12	GET_PIN(F, 11)
#define GPIO_MI13	GET_PIN(F, 12)
#define GPIO_MI14	GET_PIN(F, 13)
#define GPIO_MI15	GET_PIN(F, 14)
#define GPIO_MI16	GET_PIN(F, 15)

#define GPIO_X_EN	GET_PIN(D, 10)
#define GPIO_Y_EN	GET_PIN(D, 11)
#define GPIO_Z_EN	GET_PIN(E, 10)

#define GPIO_X_ALM	GET_PIN(E, 15)
#define GPIO_Y_ALM	GET_PIN(E, 12)
#define GPIO_Z_ALM	GET_PIN(C, 13)



const int gpio_input_list[] = {
GPIO_MI1	,
GPIO_MI2	,
GPIO_MI3	,
GPIO_MI4	,
GPIO_MI5	,
GPIO_MI6	,
GPIO_MI7	,
GPIO_MI8	,
GPIO_MI9	,
GPIO_MI10	,
GPIO_MI11	,
GPIO_MI12	,
GPIO_MI13	,
GPIO_MI14	,
GPIO_MI15	,
GPIO_MI16
};

const int gpio_output_list[] = {
GPIO_MO1	,
GPIO_MO2	,
GPIO_MO3	,
GPIO_MO4	,
GPIO_MO5	,
GPIO_MO6	,
GPIO_MO7	,
GPIO_MO8	,
GPIO_MO9	,
GPIO_MO10	,
GPIO_MO11	,
GPIO_MO12	,
GPIO_MO13	,
GPIO_MO14	,
GPIO_MO15	,
GPIO_MO16
};

const int gpio_axis_en_list[] = {
GPIO_X_EN	,
GPIO_Y_EN	,
GPIO_Z_EN
};

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


int pin_setting()
{
    rt_pin_mode(GPIO_MI1, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(GPIO_MI2, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(GPIO_MI3, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(GPIO_MI4, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(GPIO_MI5, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(GPIO_MI6, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(GPIO_MI7, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(GPIO_MI8, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(GPIO_MI9, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(GPIO_MI10, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(GPIO_MI11, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(GPIO_MI12, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(GPIO_MI13, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(GPIO_MI14, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(GPIO_MI15, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(GPIO_MI16, PIN_MODE_INPUT_PULLUP);

    rt_pin_mode(GPIO_X_EN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(GPIO_Y_EN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(GPIO_Z_EN, PIN_MODE_INPUT_PULLUP);

    rt_pin_mode(GPIO_MO1, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_MO2, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_MO3, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_MO4, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_MO5, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_MO6, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_MO7, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_MO8, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_MO9, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_MO10, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_MO11, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_MO12, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_MO13, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_MO14, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_MO15, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_MO16, PIN_MODE_OUTPUT);

    rt_pin_mode(GPIO_X_ALM, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_Y_ALM, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_Z_ALM, PIN_MODE_OUTPUT);
    return 0;
}


void axis_enable(int index , int enable)
{
	RT_ASSERT( index > 0 && index < 3) ;
	rt_pin_write(gpio_axis_en_list[index], enable) ;
}

void gpio_output(int index , int value)
{
	int num = sizeof(gpio_output_list) / sizeof(int);
	if(index < 0 && index < num)
	{
		rt_pin_write(gpio_output_list[index], value) ;
	}
}

GPIO_Port gpio_port;

void read_all_pin(GPIO_Port * port)
{
    port->input.bit0 = !rt_pin_read(GPIO_MI1);
    port->input.bit1 = !rt_pin_read(GPIO_MI2);
    port->input.bit2 = !rt_pin_read(GPIO_MI3);
    port->input.bit3 = !rt_pin_read(GPIO_MI4);
    port->input.bit4 = !rt_pin_read(GPIO_MI5);
    port->input.bit5 = !rt_pin_read(GPIO_MI6);
    port->input.bit6 = !rt_pin_read(GPIO_MI7);
    port->input.bit7 = !rt_pin_read(GPIO_MI8);
    port->input.bit8 = !rt_pin_read(GPIO_MI9);
    port->input.bit9 = !rt_pin_read(GPIO_MI10);
    port->input.bit10 = !rt_pin_read(GPIO_MI11);
    port->input.bit11 = !rt_pin_read(GPIO_MI12);
    port->input.bit12 = !rt_pin_read(GPIO_MI13);
    port->input.bit13 = !rt_pin_read(GPIO_MI14);
    port->input.bit14 = !rt_pin_read(GPIO_MI15);
    port->input.bit15 = !rt_pin_read(GPIO_MI16);

    port->axis[0].enable = !rt_pin_read(GPIO_X_EN);
    port->axis[1].enable = !rt_pin_read(GPIO_Y_EN);
    port->axis[2].enable = !rt_pin_read(GPIO_Z_EN);

}

static GPIO_Port port_pre, port_in;

#define ALARM_MIN_UPLOAD_TIME 1
#define ALARM_MAX_UPLOAD_TIME 1000
void gpio_entry(void *parameters)
{
    pin_setting();
    rt_bool_t stabled = 0;
    rt_tick_t sensor_tick = rt_tick_get();
    extern void uploadSensorsData(void);
    for (;;)
    {
        //rt_sem_take(&gpio_sem, RT_WAITING_FOREVER);
        do
        {
        	port_pre.input.word = port_in.input.word;
            read_all_pin(&port_in);
            stabled = (port_pre.input.word == port_in.input.word) ;
            if (!stabled)
            {
                rt_thread_mdelay(1);
            }

        }
        while (!stabled);

        if(gpio_port.input.word != port_in.input.word){
        	gpio_port.input.word = port_in.input.word;
        	//uploadSensorsData();
        }

        if (rt_tick_get() - sensor_tick > ALARM_MAX_UPLOAD_TIME)
        {
            sensor_tick = rt_tick_get();
            //uploadSensorsData();
        }
        rt_thread_mdelay(1);
    }
}







