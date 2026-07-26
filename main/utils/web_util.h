#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t web_init(uint16_t port);

esp_err_t web_start(void);

bool web_health(void);

esp_err_t web_stop(void);

#ifdef __cplusplus
}
#endif
