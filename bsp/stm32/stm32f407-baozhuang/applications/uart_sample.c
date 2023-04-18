/*
 * 程序清单：这是一个 串口 设备使用例程
 * 例程导出了 uart_sample 命令到控制终端
 * 命令调用格式：uart_sample uart2
 * 命令解释：命令第二个参数是要使用的串口设备名称，为空则使用默认的串口设备
 * 程序功能：通过串口输出字符串"hello RT-Thread!"，然后错位输出输入的字符
*/

#include <rtthread.h>
#include <rtdevice.h>
#include "definitions.h"
#define SAMPLE_UART_NAME  "uart2"


/* 用于接收消息的信号量 */
struct rt_semaphore rx_sem;

static rt_device_t serial;

/* 接收数据回调函数 */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    /* 串口接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    rt_sem_release(&rx_sem);

    return RT_EOK;
}

extern int adc_voltage[7];  //要发送的模拟量的值
uint16_t voltage_recv = 0;  //电压接收值
uint16_t current_recv = 0;  //电流设定值
uint8_t percentage_recv = 0;  //百分比
extern int run_record;
extern int voltage_set,current_set;  //设定值

//Addr. SER对应串口访问寄存器的ROM地址，ADDR代表串口对应MU150的RAM地址
static void serial_thread_entry(void *parameter)
{
    unsigned char ch;
		int start = 0;  //表示上位机开始传输控制指令
    while (1)
    {
        /* 从串口读取一个字节的数据，没有读取到则等待接收信号量 */
        while (rt_device_read(serial, -1, &ch, 1) != 1)
        {
            /* 阻塞等待接收信号量，等到信号量后再次读取数据 */
            rt_sem_take(&rx_sem, RT_WAITING_FOREVER);
        }
        /* 读取到的数据通过串口错位输出 */
				if(ch == 0x01) //代表上位机要读取从机的7路AD数值
				{
					//发送的格式为先发送AD0~AD6的高8位，再发送低8位
					for(int i = 0;i < 7;i++){
							ch = (((unsigned char)adc_voltage[i]) & 0xff00) >> 8;
							rt_device_write(serial, 0, &ch, 1);
							ch = ((unsigned char)adc_voltage[i]) & 0x00ff;
							rt_device_write(serial, 0, &ch, 1);
					}
				}
				if(ch == 0x02 && start == 0)  //代表上位机要发送指定的电压值，电流值和比例（默认为100％）
				{
						start = 1;
				}
				if(start == 1) //开始接收电压高8位
				{
						voltage_recv = ch<<8;
						start++;
				}
				if(start == 2) //开始接收电压低8位
				{
						voltage_recv += ch;
						start++;
				}
				if(start == 3) //开始接收电流高8位
				{
						current_recv = ch<<8;
						start++;
				}
				if(start == 4) //开始接收电流低8位
				{
						current_recv += ch;
						start = 0;
						if(run_record == 0) //如果还没启动
						{
								voltage_set = voltage_recv;
								current_set = current_recv;
								para_write(); //数据存储
						}
				}
    }
}

int uart_sample()
{
    rt_err_t ret = RT_EOK;
    char uart_name[RT_NAME_MAX];
//    char str[] = "hello RT-Thread!\r\n";

    rt_strncpy(uart_name, SAMPLE_UART_NAME, RT_NAME_MAX);

    /* 查找系统中的串口设备 */
    serial = rt_device_find(uart_name);
    if (!serial)
    {
        rt_kprintf("find %s failed!\n", uart_name);
        return RT_ERROR;
    }

    /* 初始化信号量 */
    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);	
    /* 以中断接收及轮询发送模式打开串口设备 */
    rt_device_open(serial, RT_DEVICE_FLAG_INT_RX);
    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(serial, uart_input);
    /* 发送字符串 */
//    rt_device_write(serial, 0, str, (sizeof(str) - 1));

    /* 创建 serial 线程 */
    rt_thread_t thread = rt_thread_create("serial", serial_thread_entry, RT_NULL, 1024, 25, 10);
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
INIT_APP_EXPORT(uart_sample);
