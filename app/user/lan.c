

#include <os_type.h>
#include <c_types.h>
#include <mem.h>
#include <osapi.h>
#include <user_interface.h>
#include <smartconfig.h>

#include <spi_flash.h>
#include "driver/cJSON.h"

#include "at_custom.h"
#include "espush.h"
#include "sha1.h"
#include "websocketd.h"

#include "driver/md5.h"


#define LAN_CONTROL_PORT 6628
#define FRAME_DIRECTION_REQ "req"
#define FRAME_DIRECTION_RSP "rsp"
#define FRAME_FINDME_HEADER "ESPUSH"
#define FRAME_FINDME_RESPONSE "IMHERE"

static struct espconn findme_conn;
static esp_udp findme_udp;

const char* ICACHE_FLASH_ATTR md5(const char* buf, size_t length);
#define AT_DBG(fmt, ...) do {	\
		static char __debug_str__[128] = { 0 }; 	\
		os_sprintf(__debug_str__, fmt, ##__VA_ARGS__);	\
		at_response(__debug_str__);	\
	} while(0)


typedef void (*MessageHandler)(cJSON*, WSConnection*);

typedef struct {
	uint32_t method;
	MessageHandler handler;
} Handler_s;


void ICACHE_FLASH_ATTR authorize_handler(cJSON* msg, WSConnection* connection);
void ICACHE_FLASH_ATTR get_gpio_edge_handler(cJSON* msg, WSConnection* connection);
void ICACHE_FLASH_ATTR set_gpio_edge_handler(cJSON* msg, WSConnection* connection);
void ICACHE_FLASH_ATTR dht_value_handler(cJSON* msg, WSConnection* connection);
void ICACHE_FLASH_ATTR color_change_handler(cJSON* msg, WSConnection* connection);


enum METHOD_ALL{
	METHOD_AUTHORIZE=0,
	METHOD_GET_GPIO_EDGE=1,
	METHOD_SET_GPIO_EDGE=2,
	METHOD_DHT_VALUE=3,
	METHOD_COLOR_CHANGE=4
} ;


static Handler_s gl_handler_map[] = {
		{METHOD_AUTHORIZE, authorize_handler},
		{METHOD_GET_GPIO_EDGE, get_gpio_edge_handler},
		{METHOD_SET_GPIO_EDGE, set_gpio_edge_handler},
		{METHOD_DHT_VALUE, dht_value_handler},
		{METHOD_COLOR_CHANGE, color_change_handler},
};


void ICACHE_FLASH_ATTR authorize_handler(cJSON* msg, WSConnection* connection)
{
	AT_DBG("[%s] [%p] [%p]\r\n", __func__, msg, connection);
}


void ICACHE_FLASH_ATTR get_gpio_edge_handler(cJSON* msg, WSConnection* connection)
{
	AT_DBG("[%s] [%p] [%p]\r\n", __func__, msg, connection);
	//send message
	int i;
	char edge_buf[12];
	char out_buf[13] = { 0 };

	get_gpio_edge_to_buf(edge_buf);
	for(i=0; i != sizeof(edge_buf); ++i) {
		if(edge_buf[i]) {
			out_buf[i] = '1';
		} else {
			out_buf[i] = '0';
		}
	}

	cJSON* retmsg = cJSON_CreateObject();
	if(!retmsg) {
		at_response("MEMORY FAIL\r\n");
		return;
	}
	cJSON_AddStringToObject(retmsg, "direction", "rsp");
	cJSON_AddStringToObject(retmsg, "result", out_buf);
	char* sOut = cJSON_PrintUnformatted(retmsg);
	if(!sOut) {
		at_response("MEMORY FAIL\r\n");
		return;
	}

	sendWsMessage(connection, sOut, os_strlen(sOut), FLAG_FIN | OPCODE_TEXT);
}


void ICACHE_FLASH_ATTR set_gpio_edge_handler(cJSON* msg, WSConnection* connection)
{
	AT_DBG("[%s] [%p] [%p]\r\n", __func__, msg, connection);
	//get pin && edge
	cJSON* pin = cJSON_GetObjectItem(msg, "pin");
	if(!pin) {
		return;
	}
	if(pin->type != cJSON_Number) {
		return;
	}
	uint8_t pin_val = pin->valueint;

	cJSON* edge = cJSON_GetObjectItem(msg, "edge");
	if(!edge) {
		return;
	}
	if(edge->type != cJSON_Number) {
		return;
	}
	if(edge->valueint != 0 && edge->valueint != 1) {
		return;
	}
	uint8_t edge_val = edge->valueint;

	set_gpio_edge(pin_val, edge_val);

	//send rsp result ok
	cJSON* retmsg = cJSON_CreateObject();
	if(!retmsg) {
		at_response("MEMORY FAIL\r\n");
		return;
	}

	cJSON_AddStringToObject(retmsg, "direction", "rsp");
	cJSON_AddNumberToObject(retmsg, "result", 0);
	char* sOut = cJSON_PrintUnformatted(retmsg);
	if(!sOut) {
		at_response("MEMORY FAIL\r\n");
		return;
	}

	sendWsMessage(connection, sOut, os_strlen(sOut), FLAG_FIN | OPCODE_TEXT);
}


void ICACHE_FLASH_ATTR dht_value_handler(cJSON* msg, WSConnection* connection)
{
	AT_DBG("[%s] [%p] [%p]\r\n", __func__, msg, connection);
}


void ICACHE_FLASH_ATTR color_change_handler(cJSON* msg, WSConnection* connection)
{
	AT_DBG("[%s] [%p] [%p]\r\n", __func__, msg, connection);
}


/*
 * 1, Êý¾Ýjson½âÂë
 * */
void ICACHE_FLASH_ATTR onMessage(WSConnection* connection, const WSFrame* frame)
{
	int i, length;

	AT_DBG("Message, length: [%d]\r\n", frame->payloadLength);
	cJSON* root = cJSON_Parse(frame->payloadData);
	// parse direction, ignore rsp
	cJSON* direction_field = cJSON_GetObjectItem(root, "direction");
	if(!direction_field) {
		cJSON_Delete(root);
		return;
	}
//	AT_DBG("Message2, length: [%d]\r\n", frame->payloadLength);
	if(direction_field->type != cJSON_String) {
		cJSON_Delete(root);
		return;
	}
//	AT_DBG("Message3, length: [%d]\r\n", frame->payloadLength);
	if(!os_strncmp(direction_field->valuestring, FRAME_DIRECTION_RSP, os_strlen(FRAME_DIRECTION_RSP))) {
		cJSON_Delete(root);
		return;
	}

//	AT_DBG("Message4, length: [%d]\r\n", frame->payloadLength);
	// parse method
	cJSON* method_field = cJSON_GetObjectItem(root, "method");
	if(!method_field) {
		cJSON_Delete(root);
		return;
	}
//	AT_DBG("Message5, length: [%d]\r\n", frame->payloadLength);
	if(method_field->type != cJSON_Number) {
		cJSON_Delete(root);
		return;
	}
	//dispatch method
//	AT_DBG("Message6, length: [%d]\r\n", frame->payloadLength);
	length = sizeof(gl_handler_map) / sizeof(Handler_s);
	for(i=0; i != length; ++i) {
		AT_DBG("LOOP: [%d], MAP_ENUM: [%d], VALUE_INT: [%d], LENGTH: [%d]\r\n", i, gl_handler_map[i].method, method_field->valueint, length);
		if(gl_handler_map[i].method == method_field->valueint) {
			MessageHandler handler = gl_handler_map[i].handler;
			AT_DBG("Message7, length: [%d]\r\n", frame->payloadLength);
			handler(root, connection);
			AT_DBG("Message8, length: [%d]\r\n", frame->payloadLength);
			break;
		}
	}

	cJSON_Delete(root);
}


void ICACHE_FLASH_ATTR onConnection(WSConnection* connection)
{
	connection->onMessage = onMessage;
}


void ICACHE_FLASH_ATTR findme_recv_cb(void *arg, char *pdata, unsigned short length)
{
	AT_DBG("RECV FRAME: [%s], [%d]\r\n", pdata, length);
	//read cfg
	espush_cfg_s info;
	if(!read_espush_cfg(&info)) {
		AT_DBG("ESPUSH CONFIG ERROR.\r\n");
		return;
	}
	AT_DBG("read trace, [%d], [%s]\r\n", info.app_id, info.appkey);

	uint8 appid_buf[16 + 1] = { 0 };
	os_sprintf(appid_buf, "%d", info.app_id);
	AT_DBG("appid_buf: [%s]\r\n", appid_buf);

	// appid && md5(appid+appkey) && \00
	uint8 checker_buffer[16+32+1] = { 0 };
	os_sprintf(checker_buffer, "%d", info.app_id);
	os_memcpy(checker_buffer + os_strlen(appid_buf), info.appkey, sizeof(info.appkey));
	AT_DBG("BUF: [%s]\r\n", checker_buffer);

	os_memcpy(checker_buffer + os_strlen(appid_buf), md5(checker_buffer, os_strlen(checker_buffer)), 32);
	AT_DBG("BUF: [%s]\r\n", checker_buffer);
	at_response("TRACE!\r\n");

	remot_info *premot = NULL;
	if(espconn_get_connection_info(&findme_conn, &premot, 0) != ESPCONN_OK) {
		AT_DBG("get_connection_info error.\r\n");
	}

	os_memcpy(findme_conn.proto.udp->remote_ip, premot->remote_ip, 4);
	findme_conn.proto.udp->remote_port = premot->remote_port;

	AT_DBG("IP: [%d.%d.%d.%d], PORT: [%d]\r\n", premot->remote_ip[0],
			premot->remote_ip[1],
			premot->remote_ip[2],
			premot->remote_ip[3],
			premot->remote_port);
	if(!os_strcmp(pdata, checker_buffer)) {
		AT_DBG("SENT BACK1\r\n");
		espconn_sent(&findme_conn, (uint8*)FRAME_FINDME_RESPONSE, os_strlen(FRAME_FINDME_RESPONSE));
	}
}

void ICACHE_FLASH_ATTR findme_sent_cb(void* arg)
{
	AT_DBG("[%s], [%p] [%p]\n", __func__, arg, &findme_conn);
}


void ICACHE_FLASH_ATTR finding_me_init()
{
	findme_conn.type = ESPCONN_UDP;
	findme_conn.proto.udp = &findme_udp;
	findme_conn.proto.udp->local_port = 6629;
	espconn_regist_recvcb(&findme_conn, findme_recv_cb);
	espconn_regist_sentcb(&findme_conn, findme_sent_cb);

	int iRet = espconn_create(&findme_conn);
	AT_DBG("FINDING ME INIT: [%d]\r\n", iRet);
}


const char* ICACHE_FLASH_ATTR md5(const char* buf, size_t length)
{
	int i;
	char out[16];
	static char resout[32 + 1];
	MD5_CTX ctx;

	MD5_Init(&ctx);
	MD5_Update(&ctx, "HELLO", 5);
	MD5_Final(out, &ctx);

	for(i=0; i != 16; ++i) {
		os_sprintf(resout + 2 * i, "%02x", out[i]);
	}
	resout[32] = 0;

	return resout;
}


void ICACHE_FLASH_ATTR lan_control_init()
{
	AT_DBG("LAN_CONTROL_INIT\r\n");
	websocketdInit(LAN_CONTROL_PORT, onConnection);
	finding_me_init();
}

