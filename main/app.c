#include <stdio.h>
#include "dotenv.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi_connect.h"

static const char *LOGGER = "APP";

void app_main(void)
{
    dotenv_init();

    const char *ssid = dotenv_get("SSID_WIFI");
    const char *passwd = dotenv_get("PASSWD_WIFI");

    if (!ssid || !passwd) {
        ESP_LOGE(LOGGER, "SSID_WIFI or PASSWD_WIFI not set in .env");
        return;
    }

    ESP_ERROR_CHECK(wifi_init(ssid, passwd));
    ESP_ERROR_CHECK(wifi_connect());

    const char *lan_ip = check_LAN_ip();
    ESP_LOGI(LOGGER, "LAN IP: %s", lan_ip);

    while (1) {
        if (wifi_health()) {
            ESP_LOGI(LOGGER, "WiFi healthy! IP: %s", check_LAN_ip());
        } else {
            ESP_LOGW(LOGGER, "WiFi not healthy");
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
