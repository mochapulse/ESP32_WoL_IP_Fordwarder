#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t wifi_init(const char *ssid, const char *password);

esp_err_t wifi_connect(void);

esp_err_t wifi_disconnect(void);

bool wifi_health(void);

const char *check_LAN_ip(void);

#ifdef __cplusplus
}
#endif
