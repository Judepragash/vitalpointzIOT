#include "vespa_secure_api.h"

/* Common http reponse buffer to store response data */
char http_response[8192];
esp_err_t _https_resp_handler(esp_http_client_event_t *evt)
{       
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR: 
            ESP_LOGE(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED: 
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED"); 
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                //printf("%.*s \n", evt->data_len, (char*)evt->data);
                strncpy(http_response+strlen(http_response), (char*)evt->data, evt->data_len);
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

void vespa_secure_login(struct vespa_s * vespa, char * buffer)
{
    char login_api[64];
    char post_data[128];
    memset(login_api,0,sizeof(login_api));
    sprintf(login_api,"https://%s/user/authenticate",vespa->vesp_admin_url);

    memset(post_data,0,sizeof(post_data));
    sprintf(post_data, "{ \"email\": \"%s\",\"password\": \"%s\",\"provider\": \"EMAIL\"}", vespa->userid, vespa->password);   //Send the request

    /*
        Following Code is platform dependent. 
        Works on ESP8266 based on Freertos.
        To port this code to your platform, please prepare HTTP Post as per document provided by Vitalpointz.
    */

    esp_http_client_config_t config = {
        .url = "https://dapi.vitalpointztest.com",
        .event_handler = _https_resp_handler,
    };


    //Perform HTTP POST with Content-Type as application json
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_url(client, login_api);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) 
    {
        ESP_LOGI(TAG, "HTTPS Post Sucess: %d", err);
	//On Sucess we have stored payload from response in login_response buffer
    } else {
        ESP_LOGE(TAG, "HTTPS Post Failed: %d", err);
    }
    esp_http_client_cleanup(client); //Must do cleanup if init is done

    /* Fill the HTTP response data after removing HEADER into buffer*/
    strncpy(buffer, http_response, strlen(http_response));    
}

//Call auth API After login response api only
void vespa_secure_device_auth(struct vespa_s * vespa, char * buffer)
{
    char auth_api[128];
    char auth_token[512];

    memset(http_response,0,sizeof(http_response));

    memset(auth_api,0,sizeof(auth_api));
    sprintf(auth_api,"%s/device/auth/%s",vespa->http_tenant_url,vespa->device_authcode);

    memset(auth_token,0,sizeof(auth_token));
    sprintf(auth_token,"Bearer %s",vespa->http_token);

    esp_http_client_config_t config = {
        .url = auth_api,
        .event_handler = _https_resp_handler,
    };

    //Perform HTTP POST with Content-Type as application json
    ESP_LOGI(TAG, "Auth URL: '%s'", auth_api);

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_url(client, auth_api);
    esp_http_client_set_header(client, "Authorization", auth_token);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) 
    {
        ESP_LOGI(TAG, "HTTP Get Sucess: %d", err);
	//On Sucess we have stored payload from response in login_response buffer
    } else {
        ESP_LOGE(TAG, "HTTP Get Failed: %d", err);
    }
    esp_http_client_cleanup(client); //Must do cleanup if init is done
    
    /* Fill the HTTP response data after removing HEADER into buffer*/
    strncpy(buffer, http_response, strlen(http_response));
}

void vespa_secure_device_cap(struct vespa_s * vespa, char * buffer)
{
    char cap_api[256];
    char auth_token[512];
    char post_data[64];

    memset(http_response,0,sizeof(http_response));
    memset(cap_api,0,sizeof(cap_api));
    sprintf(cap_api,"%s/device/capability/%s",vespa->http_tenant_url,vespa->deviceid);
    memset(auth_token,0,sizeof(auth_token));
    sprintf(auth_token,"Bearer %s",vespa->http_token);
    memset(&post_data,0,sizeof(post_data));
    sprintf(post_data, "{ \"devicetype\": \"MicroController\"}");   //Send the request

    esp_http_client_config_t config = {
        .url = cap_api,
        .event_handler = _https_resp_handler,
    };

    //Perform HTTPS POST with Content-Type as application json
    ESP_LOGI(TAG, "Capability URL        : '%s'", cap_api);
    ESP_LOGI(TAG, "Capability Auth Token : '%s'", auth_token);
    ESP_LOGI(TAG, "Capability Data       : '%s'", post_data);

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_url(client, cap_api);
    esp_http_client_set_header(client, "Authorization", auth_token);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) 
    {
        ESP_LOGI(TAG, "HTTPS Capability POST Sucess: %d", err);
	//On Sucess we have stored payload from response in login_response buffer
    } else {
        ESP_LOGE(TAG, "HTTPS Capability POST Failed: %d", err);
    }
    esp_http_client_cleanup(client); //Must do cleanup if init is done

    /* Fill the HTTP response data after removing HEADER into buffer*/
    strncpy(buffer, http_response, strlen(http_response));
}

/***************************************************************

Following APIs are for parsing and storing.
The API's use cJSON parser library from Freertos.
Each api need input as response buffer received as prerequisite.

******************************************************************/

/* 
This api must be called after calling login API.
This api will decode and store the needed data to make next api call into vespa handle.
Arguments : VESPa handle and login api response buffer string received.
*/

void vespa_parse_login_response(struct vespa_s * vespa, char * data)
{

    char mqtt_url[64];
    const cJSON *name = NULL;
    printf("data= '%s'\n", data);
    
    cJSON *json = cJSON_Parse(data);
    if (json == NULL){
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            printf("Error before: %s\n", error_ptr);
        }
        return;
    }

    name = cJSON_GetObjectItemCaseSensitive(json, "device_tenant_api_url");
    if (cJSON_IsString(name) && (name->valuestring != NULL))
    {
        strncpy(vespa->http_tenant_url, name->valuestring, strlen(name->valuestring));
    }
    name = cJSON_GetObjectItemCaseSensitive(json, "device_tenant_mqtt_url");
    if (cJSON_IsString(name) && (name->valuestring != NULL))
    {
        strncpy(mqtt_url, name->valuestring, strlen(name->valuestring));
        // Separate MQTT URL and Port
        strcpy(vespa->vespa_mqtt.mqtt_url, strtok(mqtt_url,":"));
        vespa->vespa_mqtt.mqtt_port = atoi(strtok(NULL, "\n"));
    }

    name = cJSON_GetObjectItemCaseSensitive(json, "device_tenant_hawkbit_url");
    if (cJSON_IsString(name) && (name->valuestring != NULL))
    {
        strncpy(vespa->swu_url, name->valuestring, strlen(name->valuestring));
    }
    name = cJSON_GetObjectItemCaseSensitive(json, "token");
    if (cJSON_IsString(name) && (name->valuestring != NULL))
    {
        strncpy(vespa->http_token, name->valuestring, strlen(name->valuestring));
    }
    name = cJSON_GetObjectItemCaseSensitive(json, "tenantid");
    if (cJSON_IsString(name) && (name->valuestring != NULL))
    {
        strncpy(vespa->tenantid, name->valuestring, strlen(name->valuestring));
    }
    cJSON_Delete(json);

    printf("Tenant Id         = '%s'\n", vespa->tenantid);
    printf("Tenant HTTP Url   = '%s'\n", vespa->http_tenant_url);
    printf("Device Auth token = '%s'\n", vespa->http_token);    
    printf("SWU Url           = '%s'\n", vespa->swu_url);
    printf("MQTT Url          = '%s'\n", vespa->vespa_mqtt.mqtt_url);
    printf("MQTT Port         = '%d'\n", vespa->vespa_mqtt.mqtt_port);
}


/* 
This api must be called after calling auth API.
This api will decode and store the needed data to make next api call into vespa handle.
Arguments : VESPa handle and auth api response buffer string received.
*/

void vespa_parse_auth_response(struct vespa_s * vespa, char* data)
{
    const cJSON *name = NULL;

    cJSON *json = cJSON_Parse(data);
    if (json == NULL){
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            printf("Error before: %s\n", error_ptr);
        }
        return;
    }

    name = cJSON_GetObjectItemCaseSensitive(json, "deviceid");
    if (cJSON_IsString(name) && (name->valuestring != NULL))
    {
        strncpy(vespa->deviceid, name->valuestring, strlen(name->valuestring));
    }
    name = cJSON_GetObjectItemCaseSensitive(json, "devicename");
    if (cJSON_IsString(name) && (name->valuestring != NULL))
    {
        strncpy(vespa->devicename, name->valuestring, strlen(name->valuestring));
    }
    cJSON_Delete(json);

    printf("Device Id         = '%s'\n", vespa->deviceid);
    printf("Device Name       = '%s'\n", vespa->devicename);
}

/* 
This api must be called after calling device capability API.
This api will decode and store the needed data to make next api call into vespa handle.
Arguments : VESPa handle and Device capability response buffer string received.
*/

void vespa_parse_cap_response(struct vespa_s * vespa, char* data)
{
//    printf("Devcap Response data = '%s' \n", data);

    const cJSON *name = NULL;
    cJSON *json = cJSON_Parse(data);
    if (json == NULL){
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            printf("Error before: %s\n", error_ptr);
        }
        return;
    }
    name = cJSON_GetObjectItemCaseSensitive(json, "mqtttopic");
    if (cJSON_IsString(name) && (name->valuestring != NULL))
    {
        strncpy(vespa->vespa_mqtt.mqtt_topic, name->valuestring, strlen(name->valuestring));
    }
    name = cJSON_GetObjectItemCaseSensitive(json, "client_key");
    if (cJSON_IsString(name) && (name->valuestring != NULL))
    {
        strncpy(vespa->client_key, name->valuestring, strlen(name->valuestring));
//          printf("Client Key = '%s', '%d'\n", name->valuestring, strlen(name->valuestring));
    }
    name = cJSON_GetObjectItemCaseSensitive(json, "client_crt");
    if (cJSON_IsString(name) && (name->valuestring != NULL))
    {
        strncpy(vespa->client_cert, name->valuestring, strlen(name->valuestring));
//          printf("Client Cert = '%s', '%d'\n", name->valuestring, strlen(name->valuestring));
    }
    name = cJSON_GetObjectItemCaseSensitive(json, "client_ca");
    if (cJSON_IsString(name) && (name->valuestring != NULL))
    {
        strncpy(vespa->client_ca, name->valuestring, strlen(name->valuestring));
//          printf("Client CA = '%s','%d'\n", name->valuestring, strlen(name->valuestring));
    }
    cJSON_Delete(json);

    printf("MQTT Topic  = '%s'\n", vespa->vespa_mqtt.mqtt_topic);
    printf("Client Key  = '%s'\n", vespa->client_key);
    printf("Client CA   = '%s'\n", vespa->client_ca);
    printf("Client Cert = '%s'\n", vespa->client_cert);
}

/* Fill config data into handle */
void vespa_initialize(struct vespa_s * vespa, struct vespa_config_s * config)
{
    strcpy(vespa->vesp_admin_url,config->vesp_url);
    strcpy(vespa->userid,config->userid);
    strcpy(vespa->password,config->password);
    strcpy(vespa->device_authcode,config->authcode);
}

