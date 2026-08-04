/**
 * @file    wifi_connect.c
 * @brief   Wi-Fi station lifecycle — implementation.
 *
 * State tracking:
 *   - FreeRTOS event group with a single WIFI_GOT_IP_BIT.
 *   - s_retrying flag prevents stacking redundant reconnection calls.
 *   - s_ip_str buffer holds the last known IPv4 address (dotted-decimal).
 *
 * Event flow:
 *   1. wifi_init()       — set up NVS, netif, event loop, Wi-Fi config.
 *   2. wifi_connect()    — esp_wifi_start() → WIFI_EVENT_STA_START
 *   3. event_handler()   — esp_wifi_connect() on STA_START.
 *   4. event_handler()   — xEventGroupSetBits(WIFI_GOT_IP_BIT) on IP_EVENT_STA_GOT_IP.
 *   5. wifi_health()     — checks the GOT_IP bit.
 */

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

/* ── State ────────────────────────────────────────────────────── */

#define WIFI_GOT_IP_BIT  BIT0   /**< Event-group bit: station has an IP lease */

static EventGroupHandle_t s_wifi_event_group;   /**< Owned by wifi_init() */
static bool               s_retrying;           /**< Guard against stacked retries */
static char               s_ip_str[16];         /**< Last known dotted-decimal IP */
static esp_netif_t       *s_sta_netif;          /**< Owned by wifi_init() */

/* ── Static IP helper ──────────────────────────────────────────── */

/**
 * @brief Stop DHCP on the STA netif and apply @p static_ip.
 *
 * Gateway is derived as x.y.z.1 and netmask is fixed to 255.255.255.0.
 * Invalid input is rejected with a warning and falls back to DHCP.
 *
 * @param static_ip  Dotted-decimal IPv4, or NULL/empty for DHCP.
 * @return ESP_OK always unless a netif call fails (propagated).
 */
static esp_err_t apply_static_ip(const char *static_ip)
{
    if (!static_ip || static_ip[0] == '\0') {
        return ESP_OK; /* DHCP default */
    }

    esp_ip4_addr_t ip;
    if (esp_netif_str_to_ip4(static_ip, &ip) != ESP_OK) {
        ESP_LOGW(TAG, "Invalid static IP '%s' — falling back to DHCP",
                 static_ip);
        return ESP_OK;
    }

    esp_netif_ip_info_t ip_info = { 0 };
    ip_info.ip = ip;

    char gw_str[16];
    snprintf(gw_str, sizeof(gw_str), "%d.%d.%d.1",
             esp_ip4_addr1(&ip), esp_ip4_addr2(&ip), esp_ip4_addr3(&ip));
    if (esp_netif_str_to_ip4(gw_str, &ip_info.gw) != ESP_OK ||
        esp_netif_str_to_ip4("255.255.255.0", &ip_info.netmask) != ESP_OK) {
        ESP_LOGW(TAG, "Failed to build static IP info — falling back to DHCP");
        return ESP_OK;
    }

    esp_err_t ret = esp_netif_dhcpc_stop(s_sta_netif);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop DHCP client: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_netif_set_ip_info(s_sta_netif, &ip_info);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set static IP: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Static IP configured: %s (gw " IPSTR ", netmask " IPSTR ")",
             static_ip, IP2STR(&ip_info.gw), IP2STR(&ip_info.netmask));
    return ESP_OK;
}

/* ── Event handler ─────────────────────────────────────────────── */

/**
 * @brief System event callback registered for WIFI_EVENT and IP_EVENT.
 *
 * - WIFI_EVENT_STA_START      → auto-connect.
 * - WIFI_EVENT_STA_DISCONNECTED → clear GOT_IP, reset s_retrying guard.
 * - IP_EVENT_STA_GOT_IP       → set GOT_IP, store IPv4 string, reset guard.
 */
static void event_handler(void *arg,
                          esp_event_base_t event_base,
                          int32_t event_id,
                          void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT &&
               event_id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupClearBits(s_wifi_event_group, WIFI_GOT_IP_BIT);
        s_retrying = false;
        s_ip_str[0] = '\0';
        ESP_LOGI(TAG, "Disconnected from AP");
    } else if (event_base == IP_EVENT &&
               event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(s_wifi_event_group, WIFI_GOT_IP_BIT);
        s_retrying = false;
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        snprintf(s_ip_str, sizeof(s_ip_str),
                 IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "Got IP: %s", s_ip_str);
    }
}

/* ── Public API ───────────────────────────────────────────────── */

esp_err_t wifi_init(const char *ssid, const char *password,
                    const char *static_ip)
{
    if (!ssid || !password) {
        return ESP_ERR_INVALID_ARG;
    }

    /* NVS — erase and re-init if the partition is corrupted or outdated */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        return ret;
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    s_sta_netif = esp_netif_create_default_wifi_sta();
    if (!s_sta_netif) {
        ESP_LOGE(TAG, "Failed to create default Wi-Fi station netif");
        return ESP_FAIL;
    }

    ret = apply_static_ip(static_ip);
    if (ret != ESP_OK) {
        return ret;
    }

    s_wifi_event_group = xEventGroupCreate();
    if (!s_wifi_event_group) {
        ESP_LOGE(TAG, "Failed to create event group");
        return ESP_ERR_NO_MEM;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .scan_method       = WIFI_FAST_SCAN,
            .sort_method       = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    strncpy((char *)wifi_config.sta.ssid,     ssid,
            sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, password,
            sizeof(wifi_config.sta.password) - 1);

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

    ESP_LOGI(TAG, "Connecting…");
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

    /* Guard — a connection attempt is already in flight */
    if (s_retrying) {
        return ESP_OK;
    }

    s_retrying = true;
    ESP_LOGW(TAG, "Retrying WiFi connection…");

    esp_err_t ret = esp_wifi_connect();
    if (ret == ESP_ERR_WIFI_NOT_STARTED) {
        /* Station is stopped — start it; the event handler will connect */
        ret = esp_wifi_start();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "WiFi start failed: %s", esp_err_to_name(ret));
            s_retrying = false;
            return ret;
        }
        /* Connection will be driven by WIFI_EVENT_STA_START → handler */
        return ESP_OK;
    }

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WiFi connect failed: %s", esp_err_to_name(ret));
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

const char *wifi_wait_for_ip(uint32_t timeout_ms)
{
    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_event_group, WIFI_GOT_IP_BIT,
        pdFALSE, pdTRUE, pdMS_TO_TICKS(timeout_ms));
    return (bits & WIFI_GOT_IP_BIT) ? s_ip_str : NULL;
}

const char *get_LAN_ip(void)
{
    return wifi_health() ? s_ip_str : NULL;
}
