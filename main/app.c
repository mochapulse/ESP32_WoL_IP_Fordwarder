#include "dotenv.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "web_util.h"
#include "wifi_connect.h"

static const char *TAG = "APP";

void app_main(void)
{
    dotenv_init();

    const char *app_name = dotenv_get("APP_NAME");
    if (!app_name) app_name = "ESP32_WoL";
    ESP_LOGI(TAG, "%s starting", app_name);

    const char *ssid = dotenv_get("SSID_WIFI");
    const char *passwd = dotenv_get("PASSWD_WIFI");
    if (!ssid || !passwd) {
        ESP_LOGE(TAG, "SSID_WIFI or PASSWD_WIFI not set in .env");
        return;
    }

    uint16_t web_port = (uint16_t)dotenv_get_int("WEB_PORT", 80);

    ESP_ERROR_CHECK(wifi_init(ssid, passwd));
    ESP_ERROR_CHECK(wifi_connect());

    const char *lan_ip = check_LAN_ip();
    ESP_LOGI(TAG, "LAN IP: %s", lan_ip);

    ESP_ERROR_CHECK(web_init(web_port));
    ESP_ERROR_CHECK(web_start());

    ESP_LOGI(TAG, "Web server: http://%s:%u", lan_ip, web_port);

    while (1) {
        if (!wifi_health()) {
            ESP_LOGW(TAG, "WiFi not healthy");
            wifi_retry();
        }
        if (!web_health()) {
            ESP_LOGW(TAG, "Web server not healthy, restarting");
            web_start();
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
