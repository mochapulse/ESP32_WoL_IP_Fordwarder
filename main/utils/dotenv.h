#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Loads the embedded .env file and parses all KEY=VALUE entries.
 * Call once during startup before calling dotenv_get().
 *
 * The .env file is embedded at build time via objcopy in CMakeLists.txt.
 */
void dotenv_init(void);

/*
 * Returns the value for `key`, or NULL if not found.
 * The returned pointer is valid for the lifetime of the program.
 */
const char *dotenv_get(const char *key);

int dotenv_get_int(const char *key, int default_value);

#ifdef __cplusplus
}
#endif
