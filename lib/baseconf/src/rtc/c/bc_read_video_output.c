#include "../internal.h"

bc_status_t bc_read_video_output(bc_video_output_t *output) {
    bc_rtc_window_state_t state;
    uint8_t modes;
    bc_status_t status;

    if (!output) return BC_INVALID_ARGUMENT;
    status = bc__rtc_window_begin(BC_AVR_WINDOW_MODES, &state);
    if (status != BC_OK) return status;
    modes = bc__rtc_read_scoped(BC_RTC_EXTENSION_INDEX);
    bc__rtc_window_end(&state);

    output->raster = (bc_raster_code_t)((modes >> 4) & 0x03u);
    output->vga = (uint8_t)(modes & 0x01u);
    output->tape_out = (uint8_t)((modes >> 1) & 0x01u);
    output->caps_led = (uint8_t)((modes >> 2) & 0x01u);
    return BC_OK;
}
