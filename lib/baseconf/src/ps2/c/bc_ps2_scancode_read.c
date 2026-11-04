#include <baseconf/ps2.h>
#include "../../rtc/internal.h"

bc_status_t bc_ps2_scancode_read(uint8_t *code) {
    bc_rtc_window_state_t state;
    uint8_t value;
    bc_status_t status;
    if (!code) return BC_INVALID_ARGUMENT;
    status = bc__rtc_window_begin(BC_AVR_WINDOW_PS2_LOG, &state);
    if (status != BC_OK) return status;
    value = bc__rtc_read_scoped(0xf0u);
    bc__rtc_window_end(&state);
    *code = value;
    return BC_OK;
}
