#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t wifi_init(const char *ssid, const char *password);

esp_err_t wifi_connect(void);

esp_err_t wifi_disconnect(void);

bool wifi_health(void);

esp_err_t wifi_retry(void);

const char *check_LAN_ip(void);

const char *wifi_wait_for_ip(uint32_t timeout_ms);

const char *get_LAN_ip(void);

#ifdef __cplusplus
}
#endif
