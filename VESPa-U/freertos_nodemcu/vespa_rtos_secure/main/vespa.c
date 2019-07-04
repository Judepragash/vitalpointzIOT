#include "vespa_secure_api.h"
#include "mqtt_client.h"
#include "driver/gpio.h"

#define MAX_RESP_BUFFER 8193
#define BLINK_GPIO 2

esp_mqtt_client_handle_t mqtt_handle;
char vespa_topic[64];
/* MQTT connection state */
int mqtt_state = 0;

/* Update this with user provided information from vesp platform */
vespa_config_t vespa_config; //User input vespa config
void user_config_init (){
    /* Fill user input data needed for vespa init. */
    strcpy(vespa_config.vesp_url,"dapi.<URL>");
    strcpy(vespa_config.userid,"username");
    strcpy(vespa_config.password,"password");
    strcpy(vespa_config.authcode,"deviceauthcode");
    /* User provided unique device hash.
       It may be device mac address or other unique device identifier. */
    strcpy(vespa_config.hash,"unique device hash");
}

/* Following is dummy sample sensor data.
   Sent to vesp cloud in with randomized values for each message
*/
typedef struct vespa_sense{
    int humidity;
    int light;
    int sound;
    int pressure;
    int temprature;
    int wind;
    float latitude;
    float longitude;
}sensor_data_t;

/* Randomize Sample sensor data */
void get_random_sensor_data(sensor_data_t * sensor_data){
    sensor_data->temprature = 20 + (esp_random() % 15); //16-38C
    sensor_data->wind = 4 + (esp_random() % 6); //1-12 m/sec
    sensor_data->humidity = 40 + (esp_random() % 9) ; //%age
    sensor_data->pressure = 900 + (esp_random() % 25);//hpa
    sensor_data->light = (esp_random() % 500) + 600; //700-1000 lux
    sensor_data->sound = (esp_random() % 3000) + 2000; //2000 - 5000 dB
    //no change in geo coordinates ..
    sensor_data->latitude = 12.994261;
    sensor_data->longitude = 77.660916;
}

/* 
   Following is reference MQTT processing logic based on ESP provided sample MQTT library and examples.
   Track mqtt Event for connection failure or other errors 
   Subscriber is done once connection state is connected.
   Event handler receives MQTT messages on MQTT_EVENT_DATA eventid.
   Following example code process only jSON messages.
*/
static esp_err_t vespa_mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    int msg_id = 0;
    int msg_type = 0;
    char sub_topic[64];
    char pub_topic[64];
    char dev_topic[64];
    char mqtt_resp[64];
    char commandid[64];
    memset(mqtt_resp,0,sizeof(mqtt_resp));
    memset(commandid,0,sizeof(commandid));
    memset(sub_topic,0,sizeof(sub_topic));
    memset(pub_topic,0,sizeof(pub_topic));
    sprintf(sub_topic,"%s/config/vespa",vespa_topic);
    sprintf(pub_topic,"%s/config/vesp",vespa_topic);
    sprintf(dev_topic,"%s/deviceled",vespa_topic);

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            mqtt_state = 1;
            msg_id = esp_mqtt_client_subscribe(mqtt_handle, sub_topic, 0);
            ESP_LOGI(TAG, "Subscribe successfully on topic='%s', msg_id=%d", sub_topic, msg_id);
            msg_id = esp_mqtt_client_subscribe(mqtt_handle, dev_topic, 0);
            ESP_LOGI(TAG, "Subscribe successfully on topic='%s', msg_id=%d", sub_topic, msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            mqtt_state = 0;
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED");
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED");
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA = '%s'", event->data);

            const cJSON *name = NULL;
            cJSON *json = cJSON_Parse(event->data);
            if (json == NULL){
                const char *error_ptr = cJSON_GetErrorPtr();
                if (error_ptr != NULL)
                {
                    printf("Error before: %s\n", error_ptr);
                }
                break;
            }
            name = cJSON_GetObjectItemCaseSensitive(json, "type");
            if (cJSON_IsString(name) && (name->valuestring != NULL))
            {
                msg_type = atoi(name->valuestring);
            }

            /* Restart ESP based on event from vesp platform
               Send SUCCESS on event receive before restarting device.
            */
            if(msg_type == 20){
                printf("Reboot Message reveived.\n");

                cJSON *cmdId = NULL;
                cJSON *item = cJSON_GetObjectItemCaseSensitive(json,"data");
                cmdId = cJSON_GetObjectItemCaseSensitive(item, "cmdId");
                if (cJSON_IsString(cmdId) && (cmdId->valuestring != NULL))
                {
                    strcpy(commandid, cmdId->valuestring);
                    printf("CommandId = '%s'\n", commandid);
                }
                sprintf(mqtt_resp,"{\"type\":\"21\",\"data\":{\"cmdId\":\"%s\",\"status\":\"SUCCESS\"}}", commandid);
                printf("MQTT Response Topic = '%s', Message = '%s' \n", pub_topic, mqtt_resp);
                esp_mqtt_client_publish(mqtt_handle, pub_topic, mqtt_resp, 0, 0, 0);
                vTaskDelay(100);
                //Soft restart
                esp_restart();
            }

            /* NodeMCU LED turn ON/OFF logic based on MQTT message received from vesp cloud */
            name = cJSON_GetObjectItemCaseSensitive(json, "led");
            if (cJSON_IsString(name) && (name->valuestring != NULL))
            {
                if(strcmp(name->valuestring, "ON")==0)
                {
                    printf("Turning ON LED\n");
                    gpio_set_level(BLINK_GPIO, 0);
                }
                if(strcmp(name->valuestring, "OFF")==0)
                {
                    printf("Turning OFF LED\n");
                    gpio_set_level(BLINK_GPIO, 1);
                }
            }

            /* reset the event->data buffer else next command may have garbage value */
            memset(event->data,0,strlen(event->data));
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
    }
    return ESP_OK;
}

/*
    Example MQTT reference implementation based on ESP mqtt library
    Following shows the All the data needed for MQTT connection is available via vespa handle.
    MQTT URL, MQTT Port, rootCA,cert and key file for secure MQTT connection. 
*/
static void vespa_mqtt_init(vespa_t * vespa)
{

    char mqtt_uri[512];
    memset(mqtt_uri,0,sizeof(mqtt_uri));
        
    esp_mqtt_client_config_t mqtt_cfg = {
        .event_handle = vespa_mqtt_event_handler,
    };

    sprintf(mqtt_uri,"mqtts://%s:%d",vespa->vespa_mqtt.mqtt_url, vespa->vespa_mqtt.mqtt_port);
    mqtt_cfg.uri = mqtt_uri;
    mqtt_cfg.cert_pem = vespa->client_ca; //rootCA
    mqtt_cfg.client_cert_pem = vespa->client_cert; //client cert
    mqtt_cfg.client_key_pem = vespa->client_key; //client key

    printf("MQTT uri         = '%s'\n", mqtt_cfg.uri);
    mqtt_handle = esp_mqtt_client_init(&mqtt_cfg);
    strncpy(vespa_topic,vespa->vespa_mqtt.mqtt_topic,strlen(vespa->vespa_mqtt.mqtt_topic)); 

    printf("Calling mqtt_start \n");
    esp_mqtt_client_start(mqtt_handle); /* Creates mqtt task */
}

/* MQTT publish task, to send  sensor data periodically to vesp platform */
static void vespa_mqtt_task(void *pvParameters)
{
    unsigned int counter = 0;
    sensor_data_t sensor_data;
    char mqtt_data[256];

    while(1)
    {
        //Wait For the MQTT to be in connected state before trying publish.
        if(mqtt_state == 1)
        {
            counter ++; //send 128 dummy sensordata msgs at 1000 tick interval
            get_random_sensor_data(&sensor_data);
            memset(mqtt_data,0,sizeof(mqtt_data));
            sprintf(mqtt_data,"{\"msgno\":%d,\"temp\":%d,\"wind\":%d,\"pressure\":%d,\"humidity\":%d,\"light\":%d,\"sound\":%d}",counter,sensor_data.temprature, sensor_data.wind, sensor_data.pressure, sensor_data.humidity,sensor_data.light,sensor_data.sound);
            esp_mqtt_client_publish(mqtt_handle, vespa_topic, mqtt_data, 0, 0, 0);
            printf("Sending mqtt message = '%s'\n",mqtt_data);
            vTaskDelay(500);
        } else {
            //Wait longer for the connection then usual send periodicity.
            vTaskDelay(500);
        }
    }
}

/* Vespa Main task, Shows how to use VESPa APIs
   HTTPS implementation is ESP specific using ESP HTTP libraries.
   This show the sequence in which the VESPa APIs should be called.
*/
static void vespa_task(void *pvParameters)
{

    vespa_t vespa_dev; //vespa Handle
    memset(&vespa_dev,0,sizeof(vespa_t));

    /* Wait for wifi to be connected before starting vespa onboarding */
    app_wifi_wait_connected();

    /* Allocate response buffer for storing data received */
    /* Vespa Capability response is large needs min 8192 buffer size 
       It contains certificates for secure connectivity */
    char *resp_buffer = malloc(MAX_RESP_BUFFER);
    if (resp_buffer == NULL) {
        ESP_LOGE(TAG, "Cannot malloc http receive buffer");
        return;
    }

    /* Call user config init to user input, 
       this can be done via reading config from eeprom if available
     */
    user_config_init();

    /* Call VESPA init now */
    vespa_initialize(&vespa_dev, &vespa_config);
 
    ESP_LOGI(TAG, "Connected to AP, INIT Done, begin vespa onboarding");
    
    /* LOGIN API will fill the resp_buffer with http response data */
    memset(resp_buffer,0,MAX_RESP_BUFFER);
    vespa_secure_login(&vespa_dev, resp_buffer);
    printf("Device Authcode '%s' \n",vespa_dev.device_authcode);
    vespa_parse_login_response(&vespa_dev, resp_buffer); 

    /* DEVICE AUTH API will fill the resp_buffer with http response data */
    memset(resp_buffer,0,MAX_RESP_BUFFER);
    ESP_LOGI(TAG, "Device Auth Start *** ");
    vespa_secure_device_auth(&vespa_dev, resp_buffer);
    vespa_parse_auth_response(&vespa_dev, resp_buffer);

    /* DEVICE Capability API will fill the resp_buffer with http response data */
    memset(resp_buffer,0,MAX_RESP_BUFFER);
    ESP_LOGI(TAG, "Device Capability Start *** ");
    vespa_secure_device_cap(&vespa_dev, resp_buffer);
    vespa_parse_cap_response(&vespa_dev, resp_buffer);

    ESP_LOGI(TAG, "vespa onboarding completed");
    memset(resp_buffer,0,MAX_RESP_BUFFER);
    free(resp_buffer); //Free the resp buffer as all data is parsed and stored.

    /* Init Flow/Data layer now */
    vespa_mqtt_init(&vespa_dev); /* Get mqtt handle after init */

    printf("VESPa onboarding complete. Starting flow layer now.\n");
    vTaskDelete(NULL);
}

void app_main()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(BLINK_GPIO, 1);

    app_wifi_initialise(); 
    xTaskCreate(&vespa_task, "vespa_task", 16384, NULL, 5, NULL);
    /* Creating vespa mqqt task for publishing periodic sensor data */
    xTaskCreate(&vespa_mqtt_task, "vespa_mqtt_task", 2048, NULL, 5, NULL);

}

