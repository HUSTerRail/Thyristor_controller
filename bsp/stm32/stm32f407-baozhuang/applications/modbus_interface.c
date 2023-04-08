/*
 * modbus_interface.c  从机代码
 *
 *  Created on: 2022年7月7日
 *      Author: xuzq
 */
#include "rtthread.h"
#include <ulog.h>

#include "modbus_rtu_test.h"
#include "modbus.h"
#include "stdio.h"
#include "string.h"

#define LOG_LVL  LOG_LVL_DBG

#define RS485_RE GET_PIN(D, 4)
#define M_MAX_PARAM_REGISTER_NUM MODBUS_MAX_WRITE_REGISTERS
#define M_MAX_MONITOR_REGISTER_NUM 40

/**
 * @brief
 * @parm offset : 参数地址
 * @parm buffer : 数据缓存区，已经是偏置后的
 */
uint16_t Charging_state[7] = {0,0,0,0,0,0,0};
void sync_parms_to_buffer(int offset , uint16_t * buffer , uint16_t len)  //主机从从机读取单个寄存器数据，初始地址为0x1000
{
	RT_ASSERT(offset >= 0);
	if(offset == 0x0000)  //代表地址为0x1000+offset，当前ad通道1输出状态
	{
		buffer[0] = Charging_state[0];
	}
	if(offset == 0x0001)   //代表地址为0x1000+offset，当前ad通道2输出状态
	{
		buffer[0] = Charging_state[1];
	}
	if(offset == 0x0002)   //代表地址为0x1000+offset，当前ad通道3输出状态
	{
		buffer[0] = Charging_state[2];
	}
	if(offset == 0x0000)  //代表地址为0x1000+offset，当前ad通道4输出状态
	{
		buffer[0] = Charging_state[3];
	}
	if(offset == 0x0001)   //代表地址为0x1000+offset，当前ad通道5输出状态
	{
		buffer[0] = Charging_state[4];
	}
	if(offset == 0x0002)   //代表地址为0x1000+offset，当前ad通道6输出状态
	{
		buffer[0] = Charging_state[5];
	}
	if(offset == 0x0000)  //代表地址为0x1000+offset，当前ad通道7输出状态
	{
		buffer[0] = Charging_state[6];
	}
}

/**
 * @brief
 * @parm offset : 参数地址
 * @parm buffer : 数据缓存区，已经是偏置后的
 */
uint16_t Charging_current = 0; //充电电流
uint16_t Charging_voltage = 0; //充电电压
void sync_buffer_to_params(int offset, uint16_t *buffer, uint16_t len) {//主机向从机写入单个寄存器数据，初始地址为0x1001
	RT_ASSERT(offset >= 0);
	if(offset == 0x0010)  //代表地址为0x1000+offset
	{
		Charging_current = buffer[0];   //从485参数地址为0x1010上接收电流值的数据（字符长度为16，个数为1）
	}
	if(offset == 0x0011)
	{
		Charging_voltage = buffer[0];   //从485参数地址为0x1011上接收电流值的数据（字符长度为16，个数为1）
	}
}

static void modbus_slave_thread(void *param) {
	int rc;
    modbus_t *ctx = RT_NULL;
    modbus_mapping_t *mb_mapping = NULL;
    ctx = modbus_new_rtu("/dev/uart2", 19200, 'N', 8, 1);  //使用串口2作为485串口
    modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);
    modbus_rtu_set_rts(ctx, RS485_RE, MODBUS_RTU_RTS_UP);
    modbus_set_slave(ctx, 1);  //设置从机地址为1
    modbus_connect(ctx);
    modbus_set_response_timeout(ctx, 0, 1000000);
	//寄存器map初始化
	mb_mapping = modbus_mapping_new_start_address(
	        0x2000, 100, 0, 0, 0x1000, M_MAX_PARAM_REGISTER_NUM, 0, M_MAX_MONITOR_REGISTER_NUM);
	if (mb_mapping == NULL) {
		log_d("Failed to allocate the mapping: %s\n", modbus_strerror(errno));
		modbus_free(ctx);
		return ;
	}
	memset(mb_mapping->tab_registers,0,M_MAX_PARAM_REGISTER_NUM);
	//循环
	while (1) {
		static uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
		//轮询接收数据，并做相应处理
		rc = modbus_receive(ctx, query);
		if (rc > 0) {
			modbus_reply(ctx, query, rc, mb_mapping);
		} else if (rc == -1) {
			/* Connection closed by the client or error */
			log_w("modbus error: %s\n", modbus_strerror(errno));
		} else {
			log_e("unknown modbus error: %s\n", modbus_strerror(errno));
			break;
		}
	}
	modbus_mapping_free(mb_mapping);
	/* For RTU, skipped by TCP (no TCP connect) */
	modbus_close(ctx);
	modbus_free(ctx);
}

static int modbus_rtu_slave_init(void)
{
    rt_pin_mode(RS485_RE, PIN_MODE_OUTPUT);
    rt_thread_t tid;
    tid = rt_thread_create("modbus",
    		modbus_slave_thread, RT_NULL,
                           2048,
                           24, 10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);
    return RT_EOK;
}
INIT_APP_EXPORT(modbus_rtu_slave_init);
