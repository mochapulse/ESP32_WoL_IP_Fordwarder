#include "wol.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"

#define WOL_PORT          9
#define WOL_PACKET_SIZE   102
#define WOL_SYNC_BYTES    6
#define WOL_MAC_BYTES     6
#define WOL_MAC_REPEATS   16

static const char *TAG = "wol";

static bool s_is_init = false;
static uint8_t s_mac[WOL_MAC_BYTES];
static struct sockaddr_in s_target_addr;

static bool is_hex_char(char c)
{
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

static bool is_valid_mac_format(const char *mac_str)
{
    if (!mac_str || strlen(mac_str) != 17) {
        return false;
    }

    for (int i = 0; i < 17; i++) {
        if (i == 2 || i == 5 || i == 8 || i == 11 || i == 14) {
            if (mac_str[i] != ':') {
                return false;
            }
        } else if (!is_hex_char(mac_str[i])) {
            return false;
        }
    }

    return true;
}

esp_err_t wol_init(const char *wol_mac, const char *bcast_ip)
{
    if (!wol_mac || !bcast_ip) {
        ESP_LOGE(TAG, "wol_init: wol_mac or bcast_ip is NULL");
        s_is_init = false;
        return ESP_ERR_INVALID_ARG;
    }

    if (!is_valid_mac_format(wol_mac)) {
        ESP_LOGE(TAG, "wol_init: invalid MAC format: %s", wol_mac);
        s_is_init = false;
        return ESP_ERR_INVALID_ARG;
    }

    unsigned int mac_parts[WOL_MAC_BYTES];
    int matched = sscanf(wol_mac, "%2x:%2x:%2x:%2x:%2x:%2x",
                         &mac_parts[0], &mac_parts[1], &mac_parts[2],
                         &mac_parts[3], &mac_parts[4], &mac_parts[5]);
    if (matched != WOL_MAC_BYTES) {
        ESP_LOGE(TAG, "wol_init: failed parsing MAC: %s", wol_mac);
        s_is_init = false;
        return ESP_ERR_INVALID_ARG;
    }

    for (int i = 0; i < WOL_MAC_BYTES; i++) {
        s_mac[i] = (uint8_t)mac_parts[i];
    }

    memset(&s_target_addr, 0, sizeof(s_target_addr));
    s_target_addr.sin_family = AF_INET;
    s_target_addr.sin_port = htons(WOL_PORT);

    int rc = inet_pton(AF_INET, bcast_ip, &s_target_addr.sin_addr);
    if (rc != 1) {
        ESP_LOGE(TAG, "wol_init: invalid broadcast IP: %s", bcast_ip);
        s_is_init = false;
        return ESP_ERR_INVALID_ARG;
    }

    s_is_init = true;
    ESP_LOGI(TAG, "Configured target=%s mac=%s", bcast_ip, wol_mac);
    return ESP_OK;
}

esp_err_t wol_send(void)
{
    if (!s_is_init) {
        ESP_LOGE(TAG, "wol_send called before successful wol_init");
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t packet[WOL_PACKET_SIZE];
    memset(packet, 0xFF, WOL_SYNC_BYTES);

    for (int i = 0; i < WOL_MAC_REPEATS; i++) {
        memcpy(packet + WOL_SYNC_BYTES + (i * WOL_MAC_BYTES), s_mac, WOL_MAC_BYTES);
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        ESP_LOGE(TAG, "socket failed, errno=%d", errno);
        return ESP_FAIL;
    }

    int broadcast_enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST,
                   &broadcast_enable, sizeof(broadcast_enable)) < 0) {
        ESP_LOGE(TAG, "setsockopt(SO_BROADCAST) failed, errno=%d", errno);
        close(sockfd);
        return ESP_FAIL;
    }

    ssize_t sent = sendto(sockfd, packet, sizeof(packet), 0,
                          (const struct sockaddr *)&s_target_addr,
                          sizeof(s_target_addr));
    if (sent < 0) {
        ESP_LOGE(TAG, "sendto failed, errno=%d", errno);
        close(sockfd);
        return ESP_FAIL;
    }

    close(sockfd);
    ESP_LOGI(TAG, "Magic packet sent");
    return ESP_OK;
}