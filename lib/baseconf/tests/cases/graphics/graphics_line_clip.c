#include <baseconf.h>
#include <baseconf/graphics.h>
#include <graphics.h>
#include <stdint.h>

#include "bc_test.h"

static void check_x_major(uint8_t expected) {
    int x;
    for (x = 0; x < 256; ++x) {
        int y = x >> 1;
        BC_ASSERT(point(x, y) == expected);
        BC_ASSERT(point(x, y + 1) == 0);
    }
}

int main(void) {
    int y;

    if (bc_test_init() != BC_OK || bc_select_screen(BC_SCREEN_0) != BC_OK ||
        bc_set_video_mode(BC_VIDEO_ZX_256) != BC_OK) {
        BC_ASSERT(0);
        bc_test_finish();
        return 0;
    }

    clg();
    /* A major delta of 65535 crosses both sides of the screen. The original
     * Bresenham raster reduces to y = x / 2 in the visible interval. */
    bc_draw_line_op(INT16_MIN, -16384, INT16_MAX, 16383, BC_DRAW_SET);
    check_x_major(1);
    xordraw(INT16_MIN, -16384, INT16_MAX, 16383);
    check_x_major(0);

    /* Transpose and reverse the endpoints to exercise descending y-major
     * seeding. In the visible interval x = y / 2. */
    bc_draw_line_op(16383, INT16_MAX, -16384, INT16_MIN, BC_DRAW_SET);
    for (y = 0; y < 192; ++y) {
        int x = y >> 1;
        BC_ASSERT(point(x, y) == 1);
        BC_ASSERT(point(x + 1, y) == 0);
    }

    bc_test_finish();
    return 0;
}
