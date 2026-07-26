#include "wifi_connect.h"

#include <stdio.h>
#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"

static const char *TAG = "wifi_connect";

#define WIFI_GOT_IP_BIT BIT0

static EventGroupHandle_t s_wifi_event_group;
static bool s_retrying;
static char s_ip_str[16];

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupClearBits(s_wifi_event_group, WIFI_GOT_IP_BIT);
        s_retrying = false;
        s_ip_str[0] = '\0';
        ESP_LOGI(TAG, "Disconnected from AP");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(s_wifi_event_group, WIFI_GOT_IP_BIT);
        s_retrying = false;
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        snprintf(s_ip_str, sizeof(s_ip_str), IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "Got IP: %s", s_ip_str);
    }
}

esp_err_t wifi_init(const char *ssid, const char *password)
{
    if (!ssid || !password) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        return ret;
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    s_wifi_event_group = xEventGroupCreate();
    if (!s_wifi_event_group) {
        ESP_LOGE(TAG, "Failed to create event group");
        return ESP_ERR_NO_MEM;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .scan_method = WIFI_FAST_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    ESP_LOGI(TAG, "WiFi init done for SSID: %s", ssid);
    return ESP_OK;
}

esp_err_t wifi_connect(void)
{
    esp_err_t ret = esp_wifi_start();
    if (ret != ESP_OK) {
        return ret;
    }

    ESP_LOGI(TAG, "Connecting...");
    return ESP_OK;
}

esp_err_t wifi_disconnect(void)
{
    esp_err_t ret = esp_wifi_disconnect();
    if (ret != ESP_OK && ret != ESP_ERR_WIFI_NOT_STARTED) {
        return ret;
    }

    ret = esp_wifi_stop();
    if (ret != ESP_OK && ret != ESP_ERR_WIFI_NOT_INIT) {
        return ret;
    }

    xEventGroupClearBits(s_wifi_event_group, WIFI_GOT_IP_BIT);
    s_retrying = false;
    s_ip_str[0] = '\0';
    ESP_LOGI(TAG, "Disconnected and stopped");
    return ESP_OK;
}

bool wifi_health(void)
{
    return (xEventGroupGetBits(s_wifi_event_group) & WIFI_GOT_IP_BIT) != 0;
}

esp_err_t wifi_retry(void)
{
    if (wifi_health()) {
        s_retrying = false;
        return ESP_OK;
    }

    if (s_retrying) {
        return ESP_OK;
    }

    s_retrying = true;
    ESP_LOGW(TAG, "Retrying WiFi connection...");
    esp_err_t ret = esp_wifi_connect();
    if (ret == ESP_ERR_WIFI_NOT_STARTED) {
        ret = esp_wifi_start();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "WiFi retry failed: %s", esp_err_to_name(ret));
            s_retrying = false;
            return ret;
        }
        return ESP_OK;
    }

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WiFi retry failed: %s", esp_err_to_name(ret));
        s_retrying = false;
    }
    return ret;
}

const char *check_LAN_ip(void)
{
    xEventGroupWaitBits(s_wifi_event_group, WIFI_GOT_IP_BIT,
                        pdFALSE, pdTRUE, portMAX_DELAY);
    return s_ip_str;
}

const char *get_LAN_ip(void)
{
    if (wifi_health()) return s_ip_str;
    return NULL;
}
