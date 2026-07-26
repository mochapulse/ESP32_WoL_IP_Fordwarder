#include "web_util.h"

#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "web_API.h"

static const char *TAG = "web_util";

static httpd_handle_t s_server = NULL;
static uint16_t s_port = 80;

esp_err_t web_init(uint16_t port)
{
    s_port = port;
    ESP_LOGI(TAG, "Initialized on port %u", s_port);
    return ESP_OK;
}

esp_err_t web_start(void)
{
    if (s_server) {
        ESP_LOGW(TAG, "Server already running");
        return ESP_OK;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = s_port;

    esp_err_t ret = httpd_start(&s_server, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start: %s", esp_err_to_name(ret));
        return ret;
    }

    web_API_init(s_server);

    ESP_LOGI(TAG, "Started on port %u", s_port);
    return ESP_OK;
}

bool web_health(void)
{
    return s_server != NULL;
}

esp_err_t web_stop(void)
{
    if (!s_server) {
        return ESP_OK;
    }

    esp_err_t ret = httpd_stop(s_server);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop: %s", esp_err_to_name(ret));
        return ret;
    }

    s_server = NULL;
    ESP_LOGI(TAG, "Stopped");
    return ESP_OK;
}
