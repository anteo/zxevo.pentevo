#include <stdint.h>
#include "zxhost.h"
#include "zxhost_config.h"

static volatile uint8_t *const mailbox = (volatile uint8_t *)(uintptr_t)ZXHOST_HEADER_ADDRESS;
static volatile uint8_t *const log_buffer = (volatile uint8_t *)(uintptr_t)ZXHOST_LOG_ADDRESS;
static uint16_t log_cursor;

void zxhost_init(void) {
    mailbox[ZXHOST_FIELD_MAGIC + 0] = ZXHOST_MAGIC_0;
    mailbox[ZXHOST_FIELD_MAGIC + 1] = ZXHOST_MAGIC_1;
    mailbox[ZXHOST_FIELD_MAGIC + 2] = ZXHOST_MAGIC_2;
    mailbox[ZXHOST_FIELD_MAGIC + 3] = ZXHOST_MAGIC_3;
    mailbox[ZXHOST_FIELD_VERSION] = ZXHOST_PROTOCOL_VERSION;
    mailbox[ZXHOST_FIELD_STATE] = ZXHOST_STATE_IDLE;
    mailbox[ZXHOST_FIELD_FLAGS] = 0;
    mailbox[ZXHOST_FIELD_RESULT0] = 0;
    mailbox[ZXHOST_FIELD_RESULT1] = 0;
    mailbox[ZXHOST_FIELD_RESULT2] = 0;
    mailbox[ZXHOST_FIELD_RESULT3] = 0;
    zxhost_log_init();
}

zxhost_request_t zxhost_get_request(void) {
    zxhost_request_t request;
    request.command = mailbox[ZXHOST_FIELD_COMMAND];
    request.arg0 = mailbox[ZXHOST_FIELD_ARG0];
    request.arg1 = mailbox[ZXHOST_FIELD_ARG1];
    request.sequence = mailbox[ZXHOST_FIELD_SEQUENCE];
    return request;
}

uint8_t zxhost_cancelled(void) {
    return mailbox[ZXHOST_FIELD_CANCEL] != 0;
}

void zxhost_set_results(uint8_t result0, uint8_t result1, uint8_t result2, uint8_t result3) {
    mailbox[ZXHOST_FIELD_RESULT0] = result0;
    mailbox[ZXHOST_FIELD_RESULT1] = result1;
    mailbox[ZXHOST_FIELD_RESULT2] = result2;
    mailbox[ZXHOST_FIELD_RESULT3] = result3;
}

void zxhost_finish(uint8_t state, uint8_t flags) {
    if (state != ZXHOST_STATE_DONE && state != ZXHOST_STATE_ABORTED && state != ZXHOST_STATE_ERROR)
        state = ZXHOST_STATE_ERROR;
    mailbox[ZXHOST_FIELD_STATE] = ZXHOST_STATE_IDLE;
    mailbox[ZXHOST_FIELD_FLAGS] = flags;
    mailbox[ZXHOST_FIELD_VERSION] = ZXHOST_PROTOCOL_VERSION;
    mailbox[ZXHOST_FIELD_MAGIC + 0] = ZXHOST_MAGIC_0;
    mailbox[ZXHOST_FIELD_MAGIC + 1] = ZXHOST_MAGIC_1;
    mailbox[ZXHOST_FIELD_MAGIC + 2] = ZXHOST_MAGIC_2;
    mailbox[ZXHOST_FIELD_MAGIC + 3] = ZXHOST_MAGIC_3;
    mailbox[ZXHOST_FIELD_STATE] = state;
}

void zxhost_log_init(void) {
    uint16_t i;
    for (i = 0; i < ZXHOST_LOG_SIZE; ++i) log_buffer[i] = 0;
    log_cursor = 0;
}

void zxhost_log_char(uint8_t ch) {
    if (ch && log_cursor + 1u < ZXHOST_LOG_SIZE) {
        log_buffer[log_cursor++] = ch;
        log_buffer[log_cursor] = 0;
    }
}

void zxhost_log_text(const char *text) {
    if (!text) return;
    while (*text) zxhost_log_char((uint8_t)*text++);
}
