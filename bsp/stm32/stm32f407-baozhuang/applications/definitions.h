/*
 * definitions.h
 *
 *  Created on: 2022年3月12日
 *      Author: 徐志全
 */
#ifndef APPLICATIONS_DEFINITIONS_H_
#define APPLICATIONS_DEFINITIONS_H_
#include <rtthread.h>
#include <rtdevice.h>
#pragma anon_unions

//零点检测IO输入
//#define GPIO_PHASE_U	GET_PIN(A, 15)   
#define GPIO_PHASE_U	GET_PIN(F, 0) 
#define GPIO_PHASE_V	GET_PIN(B, 3) 
#define GPIO_PHASE_W	GET_PIN(B, 15) 

//开关信号量输入
#define GPIO_MI1	GET_PIN(F, 0)   
#define GPIO_MI2	GET_PIN(F, 1) 

//充电完成输出
#define GPIO_MO1	GET_PIN(G, 0)  

typedef struct _gpio_port {
	union {                  //声明一个共用体union
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
	}input;    //所有的输入引脚
} GPIO_Port;
int read_knob_vol_thread(void);
int pwm_led_sample();
#endif /* APPLICATIONS_DEFINITIONS_H_ */
