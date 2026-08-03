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

#include <stddef.h>

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

/**
 * @brief Parse a single "KEY=VALUE" line and store it in the entry table.
 *
 * Handles optional double-quote wrapping, leading/trailing whitespace,
 * and CRLF endings. Exposed for unit testing.
 *
 * @param line  Pointer to first non-whitespace character of the line.
 * @param len   Number of bytes until end-of-line (excluding terminator).
 */
void dotenv_parse_line(const char *line, size_t len);

/**
 * @brief Reset the internal entry table (for test isolation).
 */
void dotenv_reset(void);

#ifdef __cplusplus
}
#endif
