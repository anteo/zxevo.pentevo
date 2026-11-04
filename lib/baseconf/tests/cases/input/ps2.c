#include <baseconf.h>

#include "bc_test.h"

static uint8_t wait_scancode(void) {
    uint16_t polls = 4000;
    do {
        uint8_t code;
        if (bc_ps2_scancode_read(&code) != BC_OK) return 0;
        /* The emulator uses FF for an empty FIFO, unlike the AVR. */
        if (code != 0 && code != 0xff) return code;
    } while (--polls);
    return 0;
}

int main(void) {
    uint8_t modifiers, code;
    if (bc_test_init() == BC_OK) {
        BC_ASSERT(bc_ps2_scancode_read(&code) == BC_UNSAFE_CONTEXT);
        BC_ASSERT(bc_avr_window_select(BC_AVR_WINDOW_PS2_LOG) == BC_OK);
        BC_ASSERT(bc_ps2_scancode_read(0) == BC_INVALID_ARGUMENT);
        BC_ASSERT(bc_ps2_modifiers(0) == BC_INVALID_ARGUMENT);
        BC_ASSERT(bc_ps2_modifiers(&modifiers) == BC_OK && modifiers == 0);
        BC_ASSERT(bc_ps2_log_clear() == BC_OK);
        bc_test_step_marker("inject_a");

        BC_ASSERT(wait_scancode() == 0x1c);
        BC_ASSERT(wait_scancode() == 0xf0);
        BC_ASSERT(wait_scancode() == 0x1c);
        bc_test_step_marker("make_break");
    }
    bc_test_finish();
    return 0;
}
