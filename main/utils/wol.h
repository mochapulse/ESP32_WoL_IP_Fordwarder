/**
 * @file    wol.h
 * @brief   Wake-on-LAN utility — configuration and magic packet sender.
 */

#ifndef WOL_H
#define WOL_H

#include "esp_err.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Initialise WoL destination from string parameters.
 *
 * Validates:
 *  - MAC format: XX:XX:XX:XX:XX:XX
 *  - Broadcast IPv4 string (inet_addr compatible)
 *
 * Stores validated values in module-level state for later wol_send() calls.
 *
 * @param wol_mac    Target MAC in colon-separated hex format.
 * @param bcast_ip   Broadcast IPv4 address string (e.g. 192.168.1.255).
 * @return
 *  - ESP_OK on success.
 *  - ESP_ERR_INVALID_ARG on NULL/invalid inputs.
 */
esp_err_t wol_init(const char *wol_mac, const char *bcast_ip);

/**
 * @brief   Send a Wake-on-LAN magic packet over UDP broadcast.
 *
 * Packet layout: 6 bytes of 0xFF followed by 16 repetitions of target MAC.
 *
 * @return
 *  - ESP_OK on success.
 *  - ESP_ERR_INVALID_STATE if wol_init() has not completed successfully.
 *  - ESP_FAIL on socket/send failures.
 */
esp_err_t wol_send(void);

/**
 * @brief Check whether a character is a valid hex digit (0-9, a-f, A-F).
 *
 * @param c  Character to test.
 * @return   true if c is a hexadecimal digit.
 */
bool is_hex_char(char c);

/**
 * @brief Validate MAC address string format (XX:XX:XX:XX:XX:XX).
 *
 * @param mac_str  Null-terminated MAC address string.
 * @return         true if the format is valid.
 */
bool is_valid_mac_format(const char *mac_str);

/**
 * @brief Reset WoL module state (for test isolation).
 */
void wol_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* WOL_H */