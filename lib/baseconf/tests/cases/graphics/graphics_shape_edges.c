#include <baseconf.h>
#include <graphics.h>
#include <stdlib.h>

#include "bc_test.h"

static void check_clipped_curves(int right, int bottom) {
    int x, y;

    bc_draw_circle(0, 20, 3);
    for (y = -4; y <= 4; ++y) {
        int dy = abs(y);
        for (x = 0; x <= 4; ++x) {
            int expected = (x == 3 && dy <= 1) || (x <= 1 && dy == 3) ||
                           (x == 2 && dy == 2);
            BC_ASSERT(point(x, 20 + y) == expected);
            BC_ASSERT(point(right - x, 20 + y) == 0);
        }
    }

    bc_draw_ellipse(right, bottom, 2, 1);
    for (y = 0; y <= 2; ++y) {
        for (x = 0; x <= 3; ++x) {
            int expected = (x == 2 && y == 0) || (x <= 1 && y == 1);
            BC_ASSERT(point(right - x, bottom - y) == expected);
            BC_ASSERT(point(x, bottom - y) == 0);
        }
    }
}

static void check_degenerate_curves(int right) {
    int x, y;

    bc_draw_circle(20, 48, 0);
    bc_draw_circle(36, 48, -3);
    bc_draw_ellipse(48, 48, -2, 1);
    bc_draw_ellipse(60, 48, 2, -1);
    bc_draw_circle(-8, 48, 3);
    bc_draw_ellipse(right + 8, 48, 2, 1);
    for (y = 44; y <= 52; ++y) {
        for (x = 0; x <= 64; ++x) BC_ASSERT(point(x, y) == (x == 20 && y == 48));
        for (x = right - 8; x <= right; ++x) BC_ASSERT(point(x, y) == 0);
    }

    bc_draw_ellipse(32, 68, 0, 4);
    bc_draw_ellipse(48, 68, 4, 0);
    bc_draw_ellipse(64, 68, 0, 0);
    for (y = 63; y <= 73; ++y) {
        for (x = 31; x <= 65; ++x) {
            int expected = (x == 32 && y >= 64 && y <= 72) ||
                           (y == 68 && x >= 44 && x <= 52) || (x == 64 && y == 68);
            BC_ASSERT(point(x, y) == expected);
        }
    }
}

static void check_reversed_axes(int right, int bottom) {
    int i;
    bc_draw_hline(12, -4, 104);
    bc_draw_vline(right - 12, bottom + 4, bottom - 12);
    for (i = 0; i <= 13; ++i) {
        BC_ASSERT(point(i, 104) == (i <= 12));
        BC_ASSERT(point(right - 12, bottom - i) == (i <= 12));
    }
    BC_ASSERT(point(right, 104) == 0);
    BC_ASSERT(point(right - 12, 0) == 0);
}

static void check_shorthands(void) {
    bc_draw_line(12, 28, 16, 32);
    bc_draw_line_xor(12, 28, 16, 32);
    BC_ASSERT(point(14, 30) == 0);
    bc_draw_line_xor(12, 28, 16, 32);
    BC_ASSERT(point(14, 30) == 1);
    bc_draw_line_clear(12, 28, 16, 32);
    BC_ASSERT(point(14, 30) == 0);

    bc_draw_hline(16, 12, 30);
    bc_draw_hline_xor(16, 12, 30);
    BC_ASSERT(point(14, 30) == 0);
    bc_draw_hline_xor(16, 12, 30);
    BC_ASSERT(point(14, 30) == 1 && point(14, 31) == 0);
    bc_draw_hline_clear(16, 12, 30);
    BC_ASSERT(point(14, 30) == 0);

    bc_draw_vline(14, 32, 28);
    bc_draw_vline_xor(14, 32, 28);
    BC_ASSERT(point(14, 30) == 0);
    bc_draw_vline_xor(14, 32, 28);
    BC_ASSERT(point(14, 30) == 1 && point(15, 30) == 0);
    bc_draw_vline_clear(14, 32, 28);
    BC_ASSERT(point(14, 30) == 0);

    bc_draw_circle(28, 30, 3);
    bc_draw_circle_xor(28, 30, 3);
    BC_ASSERT(point(31, 31) == 0 && point(31, 30) == 1);
    bc_draw_circle_xor(28, 30, 3);
    BC_ASSERT(point(31, 31) == 1 && point(28, 30) == 0);
    bc_draw_circle_clear(28, 30, 3);
    BC_ASSERT(point(31, 31) == 0 && point(31, 30) == 0);

    bc_draw_ellipse(40, 30, 2, 1);
    BC_ASSERT(point(42, 30) == 1 && point(39, 31) == 1);
    bc_draw_ellipse_xor(40, 30, 2, 1);
    BC_ASSERT(point(42, 30) == 1 && point(39, 31) == 0);
    bc_draw_ellipse_xor(40, 30, 2, 1);
    BC_ASSERT(point(39, 31) == 1);
    bc_draw_ellipse_clear(40, 30, 2, 1);
    BC_ASSERT(point(42, 30) == 0 && point(39, 31) == 0);
}

static void check_mode(bc_video_mode_t mode, const char *tag) {
    bc_status_t status = bc_set_video_mode(mode);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) {
        bc_test_step_marker(tag);
        return;
    }
    clg();
    check_shorthands();
    check_clipped_curves((int)bc_video_width() - 1, (int)bc_video_height() - 1);
    check_degenerate_curves((int)bc_video_width() - 1);
    check_reversed_axes((int)bc_video_width() - 1, (int)bc_video_height() - 1);
    bc_test_step_shot(tag);
}

int main(void) {
    if (bc_test_init() == BC_OK) {
        BC_ASSERT(bc_select_screen(BC_SCREEN_0) == BC_OK);
        check_mode(BC_VIDEO_ZX_256, "zx256");
        check_mode(BC_VIDEO_ATM_320, "atm320");
    }
    bc_test_finish();
    return 0;
}
