/*
 * �����嵥������һ�� ���� �豸ʹ������
 * ���̵����� uart_sample ��������ն�
 * ������ø�ʽ��uart_sample uart2
 * ������ͣ�����ڶ���������Ҫʹ�õĴ����豸���ƣ�Ϊ����ʹ��Ĭ�ϵĴ����豸
 * �����ܣ�ͨ����������ַ���"hello RT-Thread!"��Ȼ���λ���������ַ�
*/

#include <rtthread.h>
#include <rtdevice.h>
#include "definitions.h"
#define SAMPLE_UART_NAME  "uart2"


/* ���ڽ�����Ϣ���ź��� */
struct rt_semaphore rx_sem;

static rt_device_t serial;

/* �������ݻص����� */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    /* ���ڽ��յ����ݺ�����жϣ����ô˻ص�������Ȼ���ͽ����ź��� */
    rt_sem_release(&rx_sem);

    return RT_EOK;
}

extern int adc_voltage[7];  //Ҫ���͵�ģ������ֵ
uint16_t voltage_recv = 0;  //��ѹ����ֵ
uint16_t current_recv = 0;  //�����趨ֵ
uint8_t percentage_recv = 0;  //�ٷֱ�
extern int run_record;
extern int voltage_set,current_set;  //�趨ֵ

//Addr. SER��Ӧ���ڷ��ʼĴ�����ROM��ַ��ADDR�����ڶ�ӦMU150��RAM��ַ
static void serial_thread_entry(void *parameter)
{
    unsigned char ch;
		int start = 0;  //��ʾ��λ����ʼ�������ָ��
    while (1)
    {
        /* �Ӵ��ڶ�ȡһ���ֽڵ����ݣ�û�ж�ȡ����ȴ������ź��� */
        while (rt_device_read(serial, -1, &ch, 1) != 1)
        {
            /* �����ȴ������ź������ȵ��ź������ٴζ�ȡ���� */
            rt_sem_take(&rx_sem, RT_WAITING_FOREVER);
        }
        /* ��ȡ��������ͨ�����ڴ�λ��� */
				if(ch == 0x01) //������λ��Ҫ��ȡ�ӻ���7·AD��ֵ
				{
					//���͵ĸ�ʽΪ�ȷ���AD0~AD6�ĸ�8λ���ٷ��͵�8λ
					for(int i = 0;i < 7;i++){
							ch = (((unsigned char)adc_voltage[i]) & 0xff00) >> 8;
							rt_device_write(serial, 0, &ch, 1);
							ch = ((unsigned char)adc_voltage[i]) & 0x00ff;
							rt_device_write(serial, 0, &ch, 1);
					}
				}
				if(ch == 0x02 && start == 0)  //������λ��Ҫ����ָ���ĵ�ѹֵ������ֵ�ͱ�����Ĭ��Ϊ100����
				{
						start = 1;
				}
				if(start == 1) //��ʼ���յ�ѹ��8λ
				{
						voltage_recv = ch<<8;
						start++;
				}
				if(start == 2) //��ʼ���յ�ѹ��8λ
				{
						voltage_recv += ch;
						start++;
				}
				if(start == 3) //��ʼ���յ�����8λ
				{
						current_recv = ch<<8;
						start++;
				}
				if(start == 4) //��ʼ���յ�����8λ
				{
						current_recv += ch;
						start = 0;
						if(run_record == 0) //�����û����
						{
								voltage_set = voltage_recv;
								current_set = current_recv;
								para_write(); //���ݴ洢
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

    /* ����ϵͳ�еĴ����豸 */
    serial = rt_device_find(uart_name);
    if (!serial)
    {
        rt_kprintf("find %s failed!\n", uart_name);
        return RT_ERROR;
    }

    /* ��ʼ���ź��� */
    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);	
    /* ���жϽ��ռ���ѯ����ģʽ�򿪴����豸 */
    rt_device_open(serial, RT_DEVICE_FLAG_INT_RX);
    /* ���ý��ջص����� */
    rt_device_set_rx_indicate(serial, uart_input);
    /* �����ַ��� */
//    rt_device_write(serial, 0, str, (sizeof(str) - 1));

    /* ���� serial �߳� */
    rt_thread_t thread = rt_thread_create("serial", serial_thread_entry, RT_NULL, 1024, 25, 10);
    /* �����ɹ��������߳� */
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
