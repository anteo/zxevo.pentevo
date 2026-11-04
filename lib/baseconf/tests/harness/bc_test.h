#ifndef BC_TEST_H
#define BC_TEST_H

/*
 * BaseConf test harness for lib/baseconf.
 *
 * A test is a normal C program with main().  It runs on the BaseConf target,
 * talks to the host through the ZXHost mailbox, and records screenshots, text
 * or data at deterministic step boundaries.  One test binary can cover any
 * number of video modes and produce any number of step outputs.
 *
 * The runner (tests/run.py) discovers tests/cases/*.c, builds each one for the
 * selected profile, loads it directly through ZRCP and drives the steps.
 */

#include <baseconf.h>
#include <stdint.h>

/* Initialise the harness and the library.  Call once at the beginning of main.
 * Returns BC_OK when bc_init() succeeded; a test must still call
 * bc_test_finish() so the host receives a result. */
bc_status_t bc_test_init(void);

/* Step terminators complete the current step and wait for the host before
 * returning. `name` is optional and becomes the file tag: NULL or an empty
 * string uses "out". A marker records an assertion-only step without output. */
void bc_test_step_marker(const char *name);
void bc_test_step_shot(const char *name);

/* Finish the current step with a data payload.  The host reads length bytes
 * from the already mapped CPU address and compares a canonical hex dump with
 * the expected .txt file.  The buffer must stay mapped while the host reads it
 * (static/BSS, stack, rodata or the current screen window are fine; a window
 * that the test remaps before the host answers is not).  Length is uint16_t. */
void bc_test_step_data(const char *name, const void *data, uint16_t length);

/* Same step, but the host compares the raw bytes with a .bin golden instead of
 * a reviewable hex dump.  Use it for large buffers. */
void bc_test_step_binary(const char *name, const void *data, uint16_t length);

/* Text output channel.  Build a string with bc_test_output_* (512-byte buffer,
 * truncated on overflow) and finish the step with bc_test_step_text(); the host
 * compares the raw bytes with a .txt file. */
void bc_test_output_begin(void);
void bc_test_output(const char *text);
void bc_test_output_char(char character);
void bc_test_output_u16(uint16_t value);
void bc_test_output_hex8(uint8_t value);
void bc_test_output_hex16(uint16_t value);
void bc_test_step_text(const char *name);

/* Finish the current step without a screenshot or payload; the host records a
 * SKIP. */
void bc_test_step_skip(const char *name, const char *reason);

/* Finish the test: report the accumulated result and leave the CPU spinning. */
void bc_test_finish(void);

/* Record a failed check.  The first failure determines the step result, but
 * later failures are appended to the step log as well. */
void bc_test_fail(uint16_t line, const char *expression);

#define BC_ASSERT(condition)                                                                       \
    do {                                                                                           \
        if (!(condition)) bc_test_fail((uint16_t)__LINE__, #condition);                            \
    } while (0)

#endif
