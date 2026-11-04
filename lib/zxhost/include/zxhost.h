#ifndef ZXHOST_H
#define ZXHOST_H

#include <stdint.h>
#include "zxhost_config.h"

typedef struct {
    uint8_t command;
    uint8_t arg0;
    uint8_t arg1;
    uint8_t sequence; /* request identifier, also retained in the result header */
} zxhost_request_t;

enum {
    ZXHOST_IDLE = ZXHOST_STATE_IDLE,
    ZXHOST_DONE = ZXHOST_STATE_DONE,
    ZXHOST_ABORTED = ZXHOST_STATE_ABORTED,
    ZXHOST_ERROR = ZXHOST_STATE_ERROR
};

/* Initialize result metadata/log without changing the published request. */
void zxhost_init(void);
zxhost_request_t zxhost_get_request(void);
uint8_t zxhost_cancelled(void);
void zxhost_set_results(uint8_t result0, uint8_t result1, uint8_t result2, uint8_t result3);
void zxhost_finish(uint8_t state, uint8_t flags);

/* Bounded NUL-terminated diagnostic channel, separate from result fields. */
void zxhost_log_init(void);
void zxhost_log_char(uint8_t ch);
void zxhost_log_text(const char *text);

#endif
