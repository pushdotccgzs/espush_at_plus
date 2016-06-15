#include "osapi.h"
#include "at_custom.h"
#include "user_interface.h"
#include "at_push.h"
#include "driver/key.h"

#include "espush.h"
#include "at_board.h"
#include "dht.h"

at_funcationType at_custom_cmd[] = {
	{"+PUSH", 5, NULL, at_queryCmdPushStatus, NULL, NULL},
	{"+PUSH_DEF", 9, NULL, NULL, at_setupCmdPushRegistDef, NULL},
	{"+PUSH_CUR", 9, NULL, NULL, at_setupCmdPushRegistCur, NULL},
	{"+PUSHMSG", 8, NULL, NULL, at_setupCmdPushMessage, NULL},
	{"+PUSHCLOSE", 10, NULL, NULL, NULL, at_execUnPushRegist},
	{"+PUSH_INIT", 10, NULL, NULL, NULL, at_execPushInitial},
	{"+PUSH_FLAG", 10, NULL, NULL, NULL, at_execPushFlagSwitch},
	{"+GPIO_HIGH", 10, NULL, NULL,at_setupGPIOEdgeLow, NULL},
	{"+GPIO_LOW", 9, NULL, NULL,at_setupGPIOEdgeHigh, NULL},
	{"+N_SMC", 6, NULL, NULL, NULL, at_exec_NetworkCfgTouch},
	{"+OFFLINES", 9, NULL, NULL, NULL, at_exec_ListOfflineMsg},
	{"+ADCU", 5, NULL, at_query_ADCU, NULL, NULL},
	{"+HOSTNAME", 9, NULL, at_queryHostname, at_setupHostName, NULL},
	{"+GPIO", 5, NULL, at_query_gpio, NULL, NULL},
	{"+INFO", 5, NULL, at_queryInfo, NULL, NULL},
	{"+INTERVAL", 9, NULL, NULL, at_setupInterval, NULL},
	{"+UARTRANS", 9, NULL, NULL, NULL, at_exec_UartTrans},
	{"+PUSH_CONN", 10, NULL, NULL, NULL, at_exec_espush_init},
	{"+PUSH_SAVE", 10, NULL, NULL, NULL, at_exec_espush_save},
	{"+PUSH_APPS", 10, NULL, at_query_espush_apps, NULL, NULL},
	{"+ESPUSH_TEST", 12, NULL, at_queryServerHost, at_setupServerHost, NULL},
	// AT_PLUS PLUS
	{"+LED", 4, NULL, NULL, at_setupLed, NULL},
	{"+RED", 4, NULL, NULL, at_setupRedLed, NULL},
	{"+GREEN", 6, NULL, NULL, at_setupGreenLed, NULL},
	{"+BLUE", 5, NULL, NULL, at_setupBlueLed, NULL},
	{"+DHT", 4, NULL, at_queryReadDHT, NULL, NULL},
	{"+RELAY", 6, NULL, NULL, at_setupRelax, NULL},
};

/*
 * 系统启动射频参数初始化，如非必要不要填入内容，可设置默认启动参数加速WIFI初始化过程
 */
void ICACHE_FLASH_ATTR user_rf_pre_init(void)
{
}

/*
 * 开发板初始化函数
 * 1, 初始化按钮、红外
 * 2, 初始化DHT11
 * 3, 彩灯默认亮度调节
 * 4, LED默认关闭
 */
void ICACHE_FLASH_ATTR board_init()
{
	smc_ir_key_init();
	DHTInit(SENSOR_DHT11, 5000);

	color_led_init();
	pwm_set_duty(10, 0);   // 0~2
	pwm_set_duty(10, 1);   // 0~2
	pwm_set_duty(10, 2);   // 0~2
	pwm_start();

	gpio16_output_conf();
	gpio16_output_set(0);

	espush_rtstatus_cb(rt_status_cb_func);
}


/*
 * 系统入口函数
 */
void ICACHE_FLASH_ATTR user_init(void)
{
    char* ver = "espush.cn ";
    at_customLinkMax = 5;
    at_init();
    at_set_custom_info(ver);
    at_port_print("\r\nready\r\n");
    at_cmd_array_regist(&at_custom_cmd[0], sizeof(at_custom_cmd)/sizeof(at_funcationType));
    board_init();

    system_init_done_cb((init_done_cb_t)regist_push_from_read_flash);
}
