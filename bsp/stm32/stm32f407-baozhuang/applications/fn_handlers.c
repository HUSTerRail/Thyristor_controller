#include <rtthread.h>
#include "fn_handlers.h"

#include <rtdevice.h>
#include "drv_spi.h"
#include "drv_gpio.h"
#include "hci.h"

#define LED_KEY_PIN   GET_PIN(B,4)

#define LED_SPI_DEVICE_NAME     "spi30"

struct rt_spi_device *spi_dev_led;
int spiled_init_ok = RT_FALSE;


void rt_hw_spi_led_init_entry(void *parameter) {
	while (1) {
		rt_thread_mdelay(1);
		LedKeyTick();
		LedKeyRefresh();
	}

}

void LedOut(uint16_t data) {

	uint8_t mo = 0xff;   //mo为1时，OUT也为1
	uint16_t ddd = (data << 8) | (data >> 8);   //将16位data进行前8位和后8位的调换
	if (spiled_init_ok == RT_TRUE)
		//rt_spi_send(spi_dev_led,&ddd,2);
		rt_spi_send_then_send(spi_dev_led, &mo, 1, &ddd, 2);

}

int get_key_status(int index)      //获取按键状态
{
	return rt_pin_read(LED_KEY_PIN);
}


int rt_hw_spi_led_init(void)

{
	struct rt_spi_configuration cfg;
	rt_thread_t tid_led;
	rt_err_t res;
	Led_Init();
#define LED_CS_PIN   GET_PIN(C,11)
	rt_pin_mode(LED_CS_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(LED_KEY_PIN, PIN_MODE_INPUT_PULLUP);

	res = rt_hw_spi_device_attach("spi3", LED_SPI_DEVICE_NAME, GPIOC,
			GPIO_PIN_11);
	if (res != RT_EOK) {
		rt_kprintf("led spi30 attach failed\n");
	} else {
		rt_kprintf("led spi30 attach sucess\n");
	}

	spi_dev_led = (struct rt_spi_device*) rt_device_find(LED_SPI_DEVICE_NAME); /* 查找spi设备获取设备句柄 */
	if (!spi_dev_led) {
		rt_kprintf("spi sample run failed! can't find spi30 device!\n");
	} else {
		/* config spi */
		struct rt_spi_configuration cfg;
		cfg.data_width = 8;
		cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB; /* MSB：高位在前 */
		cfg.max_hz = 1 * 1000 * 1000; /* 50M */
		rt_spi_configure(spi_dev_led, &cfg);
		spiled_init_ok = RT_TRUE;
	}

	uint8_t buff[] = { Dig_1, Dig_2, Dig_3, Dig_4, Dig_0 };     //设置数码管每一位的字符

	LedBuffWrite(buff, 0, 5);

	tid_led = rt_thread_create("co_cfg", rt_hw_spi_led_init_entry, RT_NULL,
			1024, 20, 10);
	if (tid_led != RT_NULL)
		rt_thread_startup(tid_led);
	return 0;
}
INIT_APP_EXPORT(rt_hw_spi_led_init);

void fn01_handler(void *parameters) {

}

void fn02_handler(void *parameters) {

}

void fn03_handler(void *parameters) {

}

void fn04_handler(void *parameters) {

}

void fn05_handler(void *parameters) {

}

void fn06_handler(void *parameters) {

}

void fn07_handler(void *parameters) {

}

void fn08_handler(void *parameters) {

}

void fn09_handler(void *parameters) {

}

void fn10_handler(void *parameters) {

}

void fn11_handler(void *parameters) {

}

void fn12_handler(void *parameters) {

}

void fn13_handler(void *parameters) {

}
void fn14_handler(void *parameters) {

}
void fn15_handler(void *parameters) {

}
void fn16_handler(void *parameters) {

}
void fn17_handler(void *parameters) {

}
void fn18_handler(void *parameters) {

}
void fn19_handler(void *parameters) {

}

void fn20_handler(void *parameters) {

}
