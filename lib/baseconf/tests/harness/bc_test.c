#include <baseconf.h>
#include <intrinsic.h>
#include <stdint.h>
#include <zxhost.h>

#include "bc_test.h"

#ifndef BC_TEST_PINNED
#define BC_TEST_PINNED 0x03u
#endif
#define BC_TEST_PASS 0u
#define BC_TEST_FAIL 1u
#define BC_TEST_SKIP 2u
#define BC_TEST_FLAG_SHOT 0x01u
#define BC_TEST_FLAG_MORE 0x02u
#define BC_TEST_FLAG_DATA 0x04u
#define BC_TEST_TEXT_CAPACITY 512u

static uint8_t bc_test_sequence;
static uint8_t bc_test_step;
static uint8_t bc_test_status;
static uint8_t bc_test_fail_pending;
static uint16_t bc_test_fail_line;
static const char *bc_test_fail_expression;
static char bc_test_text_buffer[BC_TEST_TEXT_CAPACITY];
static uint16_t bc_test_text_length;

static const char *bc_test_name(const char *name) {
    return name && *name ? name : "out";
}

static void bc_test_log_u16(uint16_t value) {
    char digits[5];
    uint8_t count = 0;
    if (!value) {
        zxhost_log_char('0');
        return;
    }
    while (value && count < sizeof(digits)) {
        digits[count++] = (char)('0' + value % 10u);
        value /= 10u;
    }
    while (count) zxhost_log_char((uint8_t)digits[--count]);
}

/* Wait until the host publishes the next request, then reset the step log. */
static void bc_test_next_request(void) {
    while (zxhost_get_request().sequence == bc_test_sequence) {
    }
    bc_test_sequence = zxhost_get_request().sequence;
    zxhost_init();
}

static void bc_test_log_failure(uint8_t new_line) {
    if (!bc_test_fail_pending) return;
    if (new_line) zxhost_log_char('\n');
    zxhost_log_text("line=");
    bc_test_log_u16(bc_test_fail_line);
    zxhost_log_char(' ');
    zxhost_log_text(bc_test_fail_expression);
    bc_test_fail_pending = 0;
}

static void bc_test_complete_step(uint8_t status, uint8_t flags, const char *name) {
    zxhost_log_init();
    zxhost_log_text(bc_test_name(name));
    bc_test_log_failure(1);
    zxhost_set_results(status, bc_test_step, flags, 0);
    zxhost_finish(ZXHOST_DONE, 0);
    bc_test_next_request();
    ++bc_test_step;
}

bc_status_t bc_test_init(void) {
    bc_status_t status;
    intrinsic_di();
    /* The host seeds the first request before it starts the CPU.  Preserve its
     * sequence across zxhost_init(), which only resets results and the log. */
    bc_test_sequence = zxhost_get_request().sequence;
    zxhost_init();
    bc_test_step = 0;
    bc_test_status = BC_TEST_PASS;
    bc_test_fail_pending = 0;
    bc_ers_set_profile(BC_ERS_NONE);
    bc_set_pinned_windows((uint8_t)BC_TEST_PINNED);
    status = bc_init();
    if (status != BC_OK) {
        bc_test_status = BC_TEST_FAIL;
        zxhost_log_text("init failed");
        return status;
    }
    bc_set_border_color(0);
    bc_set_ink(7);
    bc_set_paper(0);
    return BC_OK;
}

static void bc_test_data_step(const char *name, uint16_t address, uint16_t length, uint8_t kind,
                              uint8_t flags) {
    zxhost_log_init();
    zxhost_log_text(bc_test_name(name));
    zxhost_log_text("\nDATA ");
    bc_test_log_u16(address);
    zxhost_log_char(' ');
    bc_test_log_u16(length);
    zxhost_log_char(' ');
    bc_test_log_u16(kind);
    bc_test_log_failure(1);
    zxhost_set_results(bc_test_status, bc_test_step, (uint8_t)(flags | BC_TEST_FLAG_MORE), 0);
    zxhost_finish(ZXHOST_DONE, 0);
    bc_test_next_request();
    ++bc_test_step;
}

void bc_test_step_data(const char *name, const void *data, uint16_t length) {
    bc_test_data_step(name, (uint16_t)(uintptr_t)data, length, 0, BC_TEST_FLAG_DATA);
}

void bc_test_step_binary(const char *name, const void *data, uint16_t length) {
    bc_test_data_step(name, (uint16_t)(uintptr_t)data, length, 2, BC_TEST_FLAG_DATA);
}

void bc_test_output_begin(void) {
    bc_test_text_length = 0;
    bc_test_text_buffer[0] = 0;
}

void bc_test_output_char(char character) {
    if (bc_test_text_length + 1u >= BC_TEST_TEXT_CAPACITY) return;
    bc_test_text_buffer[bc_test_text_length++] = character;
    bc_test_text_buffer[bc_test_text_length] = 0;
}

void bc_test_output(const char *text) {
    while (text && *text) bc_test_output_char(*text++);
}

void bc_test_output_u16(uint16_t value) {
    char digits[5];
    uint8_t count = 0;
    if (!value) {
        bc_test_output_char('0');
        return;
    }
    while (value && count < sizeof(digits)) {
        digits[count++] = (char)('0' + value % 10u);
        value /= 10u;
    }
    while (count) bc_test_output_char(digits[--count]);
}

void bc_test_output_hex8(uint8_t value) {
    static const char hex[] = "0123456789ABCDEF";
    bc_test_output_char(hex[(value >> 4) & 0x0Fu]);
    bc_test_output_char(hex[value & 0x0Fu]);
}

void bc_test_output_hex16(uint16_t value) {
    bc_test_output_hex8((uint8_t)(value >> 8));
    bc_test_output_hex8((uint8_t)value);
}

void bc_test_step_text(const char *name) {
    bc_test_data_step(name, (uint16_t)(uintptr_t)bc_test_text_buffer, bc_test_text_length, 1,
                      BC_TEST_FLAG_DATA);
}

void bc_test_fail(uint16_t line, const char *expression) {
    bc_test_status = BC_TEST_FAIL;
    if (!bc_test_fail_pending) {
        bc_test_fail_pending = 1;
        bc_test_fail_line = line;
        bc_test_fail_expression = expression;
    }
}

void bc_test_step_shot(const char *name) {
    bc_test_complete_step(bc_test_status, (uint8_t)(BC_TEST_FLAG_SHOT | BC_TEST_FLAG_MORE), name);
}

void bc_test_step_marker(const char *name) {
    bc_test_complete_step(bc_test_status, BC_TEST_FLAG_MORE, name);
}

void bc_test_step_skip(const char *name, const char *reason) {
    zxhost_log_init();
    zxhost_log_text(bc_test_name(name));
    bc_test_log_failure(1);
    zxhost_log_text("\nSKIP ");
    zxhost_log_text(reason);
    zxhost_set_results(BC_TEST_SKIP, bc_test_step, BC_TEST_FLAG_MORE, 0);
    zxhost_finish(ZXHOST_DONE, 0);
    bc_test_next_request();
    ++bc_test_step;
}

void bc_test_finish(void) {
    zxhost_log_text(bc_test_status == BC_TEST_PASS ? "\nPASS\n" : "\nFAIL:\n");
    bc_test_log_failure(0);
    zxhost_set_results(bc_test_status, bc_test_step, 0, 0);
    zxhost_finish(ZXHOST_DONE, 0);
    for (;;) {
    }
}
