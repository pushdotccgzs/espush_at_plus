/*
 * driver.h
 *
 *  Created on: 2015��5��16��
 *      Author: Sunday
 */

#ifndef APP_INCLUDE_PUSH_H_
#define APP_INCLUDE_PUSH_H_

#include <c_types.h>

#define ESPUSH_VERSION "20150822-master-80bd4f98"


/*
 * ��С��ת����ESP8266��С����Э��涨Ϊ�������ҪΪ�������˿�����
 */
#define my_htons(_n)  ((uint16)((((_n) & 0xff) << 8) | (((_n) >> 8) & 0xff)))
#define my_ntohs(_n)  ((uint16)((((_n) & 0xff) << 8) | (((_n) >> 8) & 0xff)))
#define my_htonl(_n)  ((uint32)( (((_n) & 0xff) << 24) | (((_n) & 0xff00) << 8) | (((_n) >> 8)  & 0xff00) | (((_n) >> 24) & 0xff) ))
#define my_ntohl(_n)  ((uint32)( (((_n) & 0xff) << 24) | (((_n) & 0xff00) << 8) | (((_n) >> 8)  & 0xff00) | (((_n) >> 24) & 0xff) ))


#define read_u32(x) my_htonl(x)
#define read_u16(x) my_htons(x)


/*
 * �ͻ�������ֵ��uint8�ͣ���������ֵ����255��������Ч��
 */
enum VERTYPE {
	VER_UNKNOWN = 0,
	VER_AT = 1,
	VER_NODEMCU = 2,
	VER_SDK = 3,
	VER_OTHER = 4,
	VER_AT_PLUS = 5,
};

/*
 * UUID
 */
typedef struct {
	uint8 _buf[16];
} UUID;
void ICACHE_FLASH_ATTR uuid_to_string(UUID* puuid, char buf[32]);
void ICACHE_FLASH_ATTR create_uuid(UUID* puuid);
void ICACHE_FLASH_ATTR show_uuid(UUID* puuid);
/*
 * ���ݻص�, pdataΪ�������ݻ�������lenΪ���ݳ��ȣ��ص��������յ��������ݺ��첽���á�
 */
typedef void(*msg_cb)(uint8* pdata, uint32 len);

/*
 * ATָ��Զ��ִ�лص�, �յ�ATָ���Ŀǰ��ͬ��ִ��
 */
typedef void(*atcmd_cb)(uint8* atcmd, uint32 len);
void ICACHE_FLASH_ATTR espush_atcmd_cb(atcmd_cb func);

/*
 * LuaԶ��ִ�лص�
 */
typedef void(*luafile_cb)(uint8* filebuf, uint32 len);
void ICACHE_FLASH_ATTR espush_luafile_cb(luafile_cb func);


/*
 * ʵʱ״̬��ȡ�ص�
 */
typedef void(*rt_status_cb)(uint32 msgid, char* key, int16_t length);
void ICACHE_FLASH_ATTR espush_rtstatus_cb(rt_status_cb func);
void ICACHE_FLASH_ATTR espush_rtstatus_ret_to_gateway(uint32 cur_msgid, const char* buf, uint8_t length);


typedef void(*custom_msg_cb)(uint32 cur_msgid, uint8 msgtype, uint16 length, uint8* buf);
void ICACHE_FLASH_ATTR espush_custom_msg_cb(custom_msg_cb func);

/*
 * appid �� appkeyΪƽ̨����ֵ
 * devid Ϊ�豸Ψһ��־�룬32�ֽڣ���ʹ��uuid�Զ�����
 * ��������֣�Ϊ��ҵ���־����ʹ�÷����SDK�Ե����豸����Ψһ��λ
 */
typedef struct push_config_t {
	uint32 appid;
	uint8 appkey[32];
	uint8 devid[32];
	enum VERTYPE vertype;
	msg_cb msgcb;
}push_config;


/*
 * ����appid��appkey��devid����Ϣ
 * ��Ϊflash�����ԣ�δ��д���Ŀռ���Ҳ�������ݵ�
 * ������Ҫ����һ��hashУ��
 * ��У��ͨ����֤�����˹�д���
 */
#define APPKEY_LENGTH 32
#define DEVID_LENGTH 32
#define SSID_MAX_LENGTH 32
#define SSID_PASSWORD_MAX_LENGTH 64
typedef uint32 HASH_CLS;
typedef struct {
	uint32 app_id;
	uint8 appkey[APPKEY_LENGTH];
	char devid[DEVID_LENGTH];
	HASH_CLS hashval;
} __attribute__ ((packed)) espush_cfg_s;;



/*
 * ����ENUM������uint8�ķ�Χ��
 */
enum CONN_STATUS {
	STATUS_CONNECTING = 0,
	STATUS_DNS_LOOKUP = 1,
	STATUS_CONNECTED = 2,
	STATUS_DISCONNECTED = 3
};


enum SECOND_BOOT {
	BOOT_UNKNOWN = 0,
	BOOT_USING = 1
};

enum BOOT_APP {
	APP_UNKNOWN = 0,
	APP_USER1 = 1,
	APP_USER2 = 2,
};

enum ESPUSH_CLOUD_CONN_MODE {
	CONN_APP = 0,
	CONN_SINGLE_DEV,
};
/*
 * flash map, 2nd boot, user app
 * flash map��ֵΪ system_get_flash_map��ֵ+1��0 ���� UNKNOWN
 * 2nd boot �� user app ʹ��enum���������ֻ��ȷ�Ͽ��������Ÿ�����
 */
typedef struct regist_info_t {
	uint8 flashmap;
	uint8 second_boot;
	uint8 boot_app;
	uint8 _pad;
}regist_info_s;


/*
 * ע�ᵽespushƽ̨��appid��appkeyΪ�豸�����ʶ���������appkeyΪ32�ֽ��ַ�����ʽ
 * vertype������ enum VERTYPE��ѡ�VER_AT��VER_NODEMCUΪ AT�̼���NodeMCU�̼�ר�ã����ÿ��ܵ��´���
 * devid����Ϊ�豸Ψһʶ���룬����ʹ��оƬ��chipid�����ж��壬������NULL����ԣ�ϵͳ���Զ�ʹ��оƬchipid��ΪΨһʶ����
 * msgcb����Ϊ�յ����ݵĻص���
 */
void ICACHE_FLASH_ATTR espush_register(uint32 appid, char appkey[32], char devid[32], enum VERTYPE type, msg_cb msgcb);

/*
 * ���豸ע��
 */
void ICACHE_FLASH_ATTR espush_single_device_init(char* devid, enum VERTYPE type, msg_cb msgcb);

/*
 * ���͵Ĺ̼�ע�����ݳ�ʼ�����ο� regist_info_s �Ķ��塣
 * ��Ҫ�ǹ̼� 2nd boot���̼�SPI_SIZE�ȡ�
 */
void ICACHE_FLASH_ATTR espush_init_regist_info(regist_info_s* info);


/*
 * ��ƽ̨�Ͽ����ӣ����ӶϿ����޷��ٴη������ݣ�Ҳ���������յ�ƽ̨��֪ͨ��Ҫ����ƽ̨��������ʹ��push_regist���µ��롣
 */
void ICACHE_FLASH_ATTR espush_unregister();

/*
 * ��������������
 */
sint8 ICACHE_FLASH_ATTR espush_msg(uint8* buf, uint16 len);
sint8 ICACHE_FLASH_ATTR espush_msg_plan(uint8* buf, uint16 len, uint32 _timestamp);

/*
 * ��������״̬, �ο� enum CONN_STATUS �Ķ��塣
 */
sint8 ICACHE_FLASH_ATTR espush_server_connect_status();

void ICACHE_FLASH_ATTR espush_network_cfg_by_smartconfig();

void ICACHE_FLASH_ATTR show_systime();

void ICACHE_FLASH_ATTR espush_set_server_host(uint32 addr);
uint32 ICACHE_FLASH_ATTR espush_get_server_host();


/*
 * ���Ӻ�ɻ�õ�ǰʱ��
 * ʹ��unixʱ�����ʾ
 * ����0 �����δ����
 * ���⣺���Ϻ��ٴζϿ���ʱ�����õ�������
 */
uint32 ICACHE_FLASH_ATTR get_timestamp();

uint8 ICACHE_FLASH_ATTR set_gpio_edge(uint8 pin, uint8 edge);

/*
 * ������Ϣ����
 * ��os_printfΨһ���������ڿ������ʱ���
 * ���ϵͳ����ʱ��
 */
#define ESP_DEBUG 1

#ifdef ESP_DEBUG
#define ESP_DBG(fmt, ...) do {	\
	show_systime();	\
	os_printf(fmt, ##__VA_ARGS__);	\
	}while(0)

#else
#define ESP_DBG
#endif


#endif /* APP_INCLUDE_PUSH_H_ */
