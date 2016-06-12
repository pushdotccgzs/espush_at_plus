
#include <c_types.h>
#include <osapi.h>
#include <at_custom.h>
#include <os_type.h>
#include <spi_flash.h>
#include <eagle_soc.h>
#include <gpio.h>
#include <pwm.h>
#include "driver/key.h"
#include "driver/gpio16.h"
#include "dht.h"
#include "espush.h"

#define PWM_GREEN_OUT_IO_MUX PERIPHS_IO_MUX_MTDI_U
#define PWM_GREEN_OUT_IO_NUM 12
#define PWM_GREEN_OUT_IO_FUNC  FUNC_GPIO12
#define PWM_GREEN_BITS	BIT12

#define PWM_RED_OUT_IO_MUX PERIPHS_IO_MUX_MTMS_U
#define PWM_RED_OUT_IO_NUM 14
#define PWM_RED_OUT_IO_FUNC  FUNC_GPIO14
#define PWM_RED_BITS	BIT14

#define PWM_BLUE_OUT_IO_MUX PERIPHS_IO_MUX_MTCK_U
#define PWM_BLUE_OUT_IO_NUM 13
#define PWM_BLUE_OUT_IO_FUNC  FUNC_GPIO13
#define PWM_BLUE_BITS	BIT13



void smartconfig_succ_func()
{
	//配置成功后，调低彩灯亮度
	pwm_set_duty(10, 0);   // 0~2
	pwm_set_duty(10, 1);   // 0~2
	pwm_set_duty(10, 2);   // 0~2
	pwm_start();
}


LOCAL void ICACHE_FLASH_ATTR btn_long_press(void)
{
	at_response("SMARTCONFIG");
	espush_network_cfg_by_smartconfig_with_callback(smartconfig_succ_func);
	//按钮长按5秒后，调整彩灯亮度到高水位

	pwm_set_duty(8000, 0);   // 0~2
	pwm_set_duty(8000, 1);   // 0~2
	pwm_set_duty(8000, 2);   // 0~2
	pwm_start();
}


LOCAL void ICACHE_FLASH_ATTR btn_short_press(void)
{
	//at_response("SHORT\r\n");
}


LOCAL void ICACHE_FLASH_ATTR ir_long_press(void)
{
	at_response("IR LONG\r\n");
}


LOCAL void ICACHE_FLASH_ATTR ir_short_press(void)
{
	at_response("IR SHORT\r\n");
}


#define NUM_BTN 2

void ICACHE_FLASH_ATTR smc_ir_key_init()
{
	static struct keys_param keys;
	static struct single_key_param *keys_param[NUM_BTN];
	// 2 是按钮，0 是红外
	keys_param[0] = key_init_single(2, PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2, btn_long_press, btn_short_press); // GOOD
	keys_param[1] = key_init_single(0, PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0, ir_long_press, ir_short_press); // GOOD
//	keys_param[0] = key_init_single(12, PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12, btn_long_press, btn_short_press); // GOOD
//	keys_param[0] = key_init_single(13, PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13, btn_long_press, btn_short_press); // BAD
//	keys_param[0] = key_init_single(14, PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14, btn_long_press, btn_short_press); // GOOD
//	keys_param[0] = key_init_single(15, PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15, btn_long_press, btn_short_press); // BAD
    keys.key_num = NUM_BTN;
    keys.single_key = keys_param;
    key_init(&keys);
}


void ICACHE_FLASH_ATTR at_queryReadDHT(uint8_t id)
{
	struct sensor_reading* dht = readDHT(0);
	uint32 temperature = dht->temperature * 100;
	uint32 humidity = dht->humidity * 100;
	char out[32] = { 0 };
	os_sprintf(out, "TEMP: [%d], HUMI: [%d]\0", temperature, humidity);
	at_response(out);

	if(dht->success) {
		at_response_ok();
	} else {
		//at_response_error();
		at_response_ok();
	}
}


void ICACHE_FLASH_ATTR at_setupLed(uint8_t id, char *pPara)
{
	++pPara;
	if(pPara[0] == '0') {
		gpio16_output_set(0);
	} else if(pPara[0] == '1') {
		gpio16_output_set(1);
	}
	at_response_ok();
}


void ICACHE_FLASH_ATTR at_setupRedLed(uint8_t id, char *pPara)
{
	uint32_t val= atoi(++pPara);
	if(!val) {
		PIN_FUNC_SELECT(PWM_RED_OUT_IO_MUX, PWM_RED_OUT_IO_FUNC);
		gpio_output_set(0, PWM_RED_BITS, 0, PWM_RED_BITS);
		at_response_ok();
		return;
	}

	pwm_set_duty(val, 1);   // 0~2
	pwm_start();
	at_response_ok();
}


void ICACHE_FLASH_ATTR at_setupGreenLed(uint8_t id, char *pPara)
{
	uint32_t val= atoi(++pPara);
	if(!val) {
		PIN_FUNC_SELECT(PWM_GREEN_OUT_IO_MUX, PWM_GREEN_OUT_IO_FUNC);
		gpio_output_set(0, PWM_GREEN_BITS, 0, PWM_GREEN_BITS);
		at_response_ok();
		return;
	}


	pwm_set_duty(val, 2);   // 0~2
	pwm_start();
	at_response_ok();
}


void ICACHE_FLASH_ATTR at_setupBlueLed(uint8_t id, char *pPara)
{
	uint32_t val= atoi(++pPara);
	if(!val) {
		PIN_FUNC_SELECT(PWM_BLUE_OUT_IO_MUX, PWM_BLUE_OUT_IO_FUNC);
		gpio_output_set(0, PWM_BLUE_BITS, 0, PWM_BLUE_BITS);
		at_response_ok();
		return;
	}


	pwm_set_duty(val, 0);   // 0~2
	pwm_start();
	at_response_ok();
}


void ICACHE_FLASH_ATTR at_setupIR(uint8_t id, char *pPara)
{

}


void ICACHE_FLASH_ATTR at_setupRelax(uint8_t id, char *pPara)
{
	uint8 val = atoi(++pPara);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
	if(val) {
		gpio_output_set(BIT5, 0, BIT5, 0);
	} else {
		gpio_output_set(0, BIT5, 0, BIT5);
	}

	at_response_ok();
}



void ICACHE_FLASH_ATTR color_led_init()
{
	uint32 io_info[][3] = {{PWM_GREEN_OUT_IO_MUX,PWM_GREEN_OUT_IO_FUNC,PWM_GREEN_OUT_IO_NUM},
			{PWM_RED_OUT_IO_MUX,PWM_RED_OUT_IO_FUNC,PWM_RED_OUT_IO_NUM},
			{PWM_BLUE_OUT_IO_MUX,PWM_BLUE_OUT_IO_FUNC,PWM_BLUE_OUT_IO_NUM},
			};

	u32 duty[3] = {600,604,634};
	pwm_init(1000, duty, 3, io_info);
}

