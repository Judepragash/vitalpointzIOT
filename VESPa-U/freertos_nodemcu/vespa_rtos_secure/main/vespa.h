#ifndef _VESPA_H_
#define _VESPA_H_

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "app_wifi.h"

#include "cJSON.h"
#include "esp_http_client.h"

#if CONFIG_SSL_USING_WOLFSSL
#include "lwip/apps/sntp.h"
#endif

#define MAX_HTTP_RECV_BUFFER 512
static const char *TAG = "VESPA";

/* Prerequisites : 
  Wifi, HTTP and MQTT libraries,
  Change max packet size for mqtt in PubSub client library #define MQTT_MAX_PACKET_SIZE 2048
*/

typedef struct vespa_config_s {
    char vesp_url[32];
    char userid[32];
    char password[32];
    char authcode[32];
} vespa_config_t;

typedef struct vespa_mqtt_s {
    char mqtt_url[64];
    char mqtt_topic[64];
    short mqtt_port;
    void * mqtt_handle;
} vespa_mqtt_t;

typedef struct vespa_s {
    char userid[32];
    char deviceid[32];
    char devicename[64];
    char device_authcode[32]; //tenantid may be used as authcode
    char tenantid[32];
    char password[32];
    char http_token[256];
    char http_data[512];
    char vesp_admin_url[32];
    char http_tenant_url[64];
    char swu_url[64];
    vespa_mqtt_t vespa_mqtt;
    char client_key[2048];
    char client_ca[2048];
    char client_cert[5120];
} vespa_t;

#endif
