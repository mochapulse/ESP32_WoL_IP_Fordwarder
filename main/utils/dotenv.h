/**
 * @file    dotenv.h
 * @brief   Embedded .env parser — stores up to 16 key/value pairs in static
 *          arrays parsed from a binary blob linked via objcopy.
 *
 * The .env file is embedded at build time by CMakeLists.txt using objcopy.
 * Symbols: `_binary__env_start` / `_binary__env_end`.
 *
 * @note   Call dotenv_init() once before any dotenv_get() / dotenv_get_int().
 *         All returned pointers are valid for the program lifetime.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Load and parse the embedded .env file.
 *
 * Strips leading/trailing whitespace, skips comments ('#' lines), and
 * handles optional double-quote wrapping on values. Unknown keys or
 * malformed lines are silently ignored.
 */
void dotenv_init(void);

/**
 * @brief Retrieve a string value by key.
 *
 * @param key  Null-terminated lookup key.
 * @return     Pointer to the value (read-only, lifetime = program), or NULL
 *             if the key was not found.
 */
const char *dotenv_get(const char *key);

/**
 * @brief Retrieve an integer value by key, falling back to a default.
 *
 * Uses `atoi()` on the stored string.
 *
 * @param key           Null-terminated lookup key.
 * @param default_value  Returned when the key is missing.
 * @return              Parsed integer or `default_value`.
 */
int dotenv_get_int(const char *key, int default_value);

#ifdef __cplusplus
}
#endif
