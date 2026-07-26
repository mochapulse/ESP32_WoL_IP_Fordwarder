#include "dotenv.h"

#include <string.h>
#include "esp_log.h"

static const char *TAG = "dotenv";

/*
 * objcopy embeds .env at link time.
 * Non-alphanumeric chars become underscore: ".env" -> "_env"
 * _binary__env_start / _binary__env_end  (double underscore from the dot)
 */
extern const unsigned char _binary__env_start[] asm("_binary__env_start");
extern const unsigned char _binary__env_end[]   asm("_binary__env_end");

#define DOTENV_MAX_KEYS  16
#define DOTENV_KEY_LEN   32
#define DOTENV_VAL_LEN   64

typedef struct {
    char key[DOTENV_KEY_LEN];
    char val[DOTENV_VAL_LEN];
} dotenv_entry_t;

static dotenv_entry_t entries[DOTENV_MAX_KEYS];
static int entry_count;

static void parse_line(const char *line, size_t len)
{
    const char *eq = memchr(line, '=', len);
    if (!eq) return;

    size_t key_len = eq - line;
    while (key_len > 0 && line[key_len - 1] == ' ') key_len--;

    const char *val = eq + 1;
    size_t val_len = len - (val - line);
    while (val_len > 0 && *val == ' ') { val++; val_len--; }

    if (val_len >= 2 && val[0] == '"' && val[val_len - 1] == '"') {
        val++;
        val_len -= 2;
    }

    while (val_len > 0 && val[val_len - 1] == ' ')  val_len--;
    while (val_len > 0 && val[val_len - 1] == '\r') val_len--;

    if (key_len == 0 || key_len >= DOTENV_KEY_LEN) return;
    if (val_len >= DOTENV_VAL_LEN) return;
    if (entry_count >= DOTENV_MAX_KEYS) return;

    memcpy(entries[entry_count].key, line, key_len);
    entries[entry_count].key[key_len] = '\0';
    memcpy(entries[entry_count].val, val, val_len);
    entries[entry_count].val[val_len] = '\0';
    entry_count++;
}

void dotenv_init(void)
{
    const unsigned char *p   = _binary__env_start;
    const unsigned char *end = _binary__env_end;
    const unsigned char *line_start = p;

    ESP_LOGI(TAG, "Loading embedded .env (%d bytes)", (int)(end - p));

    for (; p < end; p++) {
        if (*p == '\n' || *p == '\0') {
            const unsigned char *ls = line_start;
            while (ls < p && (*ls == ' ' || *ls == '\t')) ls++;
            size_t len = p - ls;
            if (len > 0 && *ls != '#') {
                parse_line((const char *)ls, len);
            }
            line_start = p + 1;
            if (*p == '\0') break;
        }
    }
    if (line_start < end) {
        const unsigned char *ls = line_start;
        while (ls < end && (*ls == ' ' || *ls == '\t')) ls++;
        size_t len = end - ls;
        if (len > 0 && *ls != '#') {
            parse_line((const char *)ls, len);
        }
    }

    ESP_LOGI(TAG, "Parsed %d entries", entry_count);
}

#include <stdlib.h>

const char *dotenv_get(const char *key)
{
    for (int i = 0; i < entry_count; i++) {
        if (strcmp(entries[i].key, key) == 0)
            return entries[i].val;
    }
    return NULL;
}

int dotenv_get_int(const char *key, int default_value)
{
    const char *val = dotenv_get(key);
    if (!val) return default_value;
    return atoi(val);
}
