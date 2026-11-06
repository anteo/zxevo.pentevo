#include <baseconf.h>
#include <baseconf/video.h>
#include <graphics.h>

#include "bc_test.h"

enum { INK = 6, CLEAR_PAPER = 4 };

static uint8_t areas_match(uint8_t cleared) {
    uint16_t x, y;
    uint8_t index;
    for (y = 47; y <= 76; ++y) {
        for (x = 0; x <= 172; ++x) {
            uint8_t expected = 0;
            if (y >= 48 && y <= 75 && (x <= 25 || (x >= 58 && x <= 97) || (x >= 132 && x <= 171)))
                expected = INK;
            if (x >= 70 && x <= 85 && y >= 56 && y <= 67) expected = cleared;
            if (x >= 141 && x <= 162 && y >= 54 && y <= 69 &&
                (x == 141 || x == 162 || y == 54 || y == 69))
                expected = cleared;
            if (bc_read_pixel_index(x, y, &index) != BC_OK || index != expected) return 0;
        }
    }
    return 1;
}

static void check_mode(bc_video_mode_t mode, const char *tag, uint8_t cleared) {
    bc_status_t status = bc_set_video_mode(mode);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) {
        bc_test_step_marker(tag);
        return;
    }
    bc_set_paper(0);
    bc_set_ink(INK);
    clg();

    /* A clipped fill, an interior cutout, and an erased frame. */
    fillb(-5, 48, 31, 28);
    fillb(32, 48, 0, 28);
    fillb(40, 48, 12, 0);
    fillb(58, 48, 40, 28);
    fillb(132, 48, 40, 28);
    bc_set_paper(CLEAR_PAPER);
    clga(85, 67, 70, 56);
    undrawb(141, 54, 22, 16);

    BC_ASSERT(areas_match(cleared));
    bc_test_step_shot(tag);
}

int main(void) {
    if (bc_test_init() == BC_OK) {
        BC_ASSERT(bc_select_screen(BC_SCREEN_0) == BC_OK);
        check_mode(BC_VIDEO_ZX_256, "zx256", 0);
        check_mode(BC_VIDEO_ATM_320, "atm320", CLEAR_PAPER);
    }
    bc_test_finish();
    return 0;
}
