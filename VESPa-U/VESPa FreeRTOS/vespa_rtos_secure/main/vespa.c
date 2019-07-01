#include "vespa_secure_api.h"
#include "mqtt_client.h"

#define MAX_RESP_BUFFER 8193

esp_mqtt_client_handle_t mqtt_handle;
char vespa_topic[64];

/* Track mqtt Event for connection failure or other errors */
static esp_err_t vespa_mqtt_event_handler(esp_mqtt_event_handle_t event)
{

    unsigned int counter = 0;
    char mqtt_data[128];
    char sensordata[64]="temprature_celcius=21,wind_mps=2,pressure_hpa=1111";

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            while(1){
                counter ++; //send 100 dummy sensordata msgs at 500 tick interval
                sprintf(mqtt_data,"msgno=%d,%s",counter,sensordata);
                if (counter <= 100){
                    esp_mqtt_client_publish(mqtt_handle, vespa_topic, sensordata, 0, 0, 0);
                    printf("Sending mqtt message = '%s'\n",mqtt_data);
                    vTaskDelay(500);
                } else {
                    break;
                }
            }
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
    }
    return ESP_OK;
}

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

static void vespa_task(void *pvParameters)
{

    /*
    vespa_t vespa_dev; //vespa Handle
    vespa_dev = malloc(sizeof(vespa_t));
    memset(&vespa_dev,0,sizeof(vespa_t));
    */

    //struct vespa_s * vespa_dev = (struct vespa_s *) (pvParameters);  //get vespa handle
    vespa_t vespa_dev; //vespa Handle
    memset(&vespa_dev,0,sizeof(vespa_t));

    vespa_config_t vespa_config; //User input config
    memset(&vespa_config,0,sizeof(vespa_config_t));

    app_wifi_wait_connected();

    /* Allocate response buffer for storing data received */
    char *resp_buffer = malloc(MAX_RESP_BUFFER);
    if (resp_buffer == NULL) {
        ESP_LOGE(TAG, "Cannot malloc http receive buffer");
        return;
    }

    /* Fill user input data needed for vespa init. */
    strcpy(vespa_config.vesp_url,"dapi.<URL>");
    strcpy(vespa_config.userid,"username");
    strcpy(vespa_config.password,"password");
    strcpy(vespa_config.authcode,"deviceauthcode");
    /* Call VESPA init now */
    vespa_initialize(&vespa_dev, &vespa_config);
 
    ESP_LOGI(TAG, "Connected to AP, INIT Done, begin vespa onboarding");
    
    ESP_LOGI(TAG, "Login Start *** ");
    /* LOGIN API will fill the resp_buffer with http response data */
    memset(resp_buffer,0,MAX_RESP_BUFFER);
    vespa_secure_login(&vespa_dev, resp_buffer);
    printf("Device Authcode '%s' \n",vespa_dev.device_authcode);
    vespa_parse_login_response(&vespa_dev, resp_buffer); 
    //strcpy(vespa_dev->device_authcode,"nNjwZSZ7Mp245pv8");


    /* DEVICE AUTH API will fill the resp_buffer with http response data */
    memset(resp_buffer,0,MAX_RESP_BUFFER);
    ESP_LOGI(TAG, "Device Auth Start *** ");
    vespa_secure_device_auth(&vespa_dev, resp_buffer);
    vespa_parse_auth_response(&vespa_dev, resp_buffer);

    //strcpy(vespa_dev->http_tenant_url,"https://dapi-5bd965d373ac9c003888f8e0.vitalpointztest.com");
    /* DEVICE Capability API will fill the resp_buffer with http response data */
    memset(resp_buffer,0,MAX_RESP_BUFFER);
    ESP_LOGI(TAG, "Device Capability Start *** ");
    vespa_secure_device_cap(&vespa_dev, resp_buffer);
    vespa_parse_cap_response(&vespa_dev, resp_buffer);

    ESP_LOGI(TAG, "vespa onboarding completed");


    free(resp_buffer); //Free the resp buffer as all data is parsed and stored.
    printf("vespa Task finish.\n");
    /* Init Flow/Data layer now */
    vespa_mqtt_init(&vespa_dev); /* Get mqtt handle after init */

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
    app_wifi_initialise();
 
    xTaskCreate(&vespa_task, "vespa_task", 16384, NULL, 5, NULL);
}

