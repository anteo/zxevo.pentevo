#include <baseconf.h>
#include <string.h>

#include "bc_test.h"

/* ERS consumes ATM color bytes starting at palette index 15. */
static const uint8_t palette[16] = {
    0x5c, 0x9f, 0x7d, 0xdf, 0xbc, 0x6f, 0xec, 0x1d,
    0x8f, 0xdc, 0x2e, 0xac, 0xcd, 0x3e, 0xfc, 0x4f,
};
static uint8_t colors[16];

int main(void) {
    uint8_t *const bitmap = (uint8_t *)0xc000u;
    uint8_t *attributes = (uint8_t *)0xd800u;
    bc_window_config_t screen = {BC_RAM, 5, 0, 0};
    bc_window_lease_t lease;
    bc_status_t status;
    uint8_t index;
    uint8_t row;

    if (bc_test_init() != BC_OK) goto finish;
    bc_ers_set_profile(BC_ERS_SERVICES_ROM);
    status = bc_ers_set_palette(palette);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) goto finish;

    status = bc_set_video_mode(BC_VIDEO_ZX_256);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) goto finish;
    status = bc_select_screen(BC_SCREEN_0);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) goto finish;
    status = bc_window_acquire(BC_WINDOW_3, screen, &lease);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) goto finish;

    /* Use the same paper and border index so readback is independent of beam position. */
    memset(bitmap, 0, 6144u);
    for (index = 0; index < sizeof(colors); ++index) {
        memset(attributes, index << 3, 768u);
        bc_set_border_color(index);
        BC_ASSERT(bc_read_beam_color(&colors[index]) == BC_OK);
    }

    /* Draw the sixteen palette entries from left to right, two cells per color. */
    for (row = 0; row < 24u; ++row) {
        for (index = 0; index < sizeof(colors); ++index) {
            *attributes++ = (uint8_t)(index << 3);
            *attributes++ = (uint8_t)(index << 3);
        }
    }
    bc_set_border_color(9); /* Gray in the test palette. */
    BC_ASSERT(bc_window_release(&lease) == BC_OK);
    bc_test_step_data("readback", colors, sizeof(colors));
    bc_test_step_shot("swatches");

finish:
    bc_test_finish();
    return 0;
}
