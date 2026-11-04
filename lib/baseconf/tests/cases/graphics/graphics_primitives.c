#include <baseconf.h>
#include <baseconf/graphics.h>
#include <baseconf/video.h>
#include <graphics.h>

#include "bc_test.h"

static void draw_mode(bc_video_mode_t mode, const char *tag) {
    int width, height, radius;
    if (bc_set_video_mode(mode) != BC_OK) {
        bc_test_step_skip(tag, "video mode unavailable");
        return;
    }
    bc_set_paper(0);
    bc_set_ink(14);
    clg();
    cplot(3, 3, 14);
    BC_ASSERT(cpoint(3, 3) == 14);
    BC_ASSERT(multipoint(0, 5, -1, 3) == 1);
    BC_ASSERT(multipoint(1, 4, 3, 2) == 4);
    BC_ASSERT((uint16_t)multipoint(0, 16, 3, 3) == 0x8000u);
    BC_ASSERT(multipoint(0, 17, 3, 3) == 0);

    /* Coincident symmetric points cancel for XOR. SET screenshots alone
     * cannot catch a change in how often those points are emitted. */
    bc_draw_ellipse_xor(12, 8, 2, 1);
    BC_ASSERT(point(11, 7) == 1);
    BC_ASSERT(point(13, 9) == 1);
    BC_ASSERT(point(10, 8) == 0);
    BC_ASSERT(point(12, 7) == 0);
    bc_draw_ellipse_xor(12, 8, 2, 1);

    xordrawb(20, 8, 3, 3);
    BC_ASSERT(point(20, 8) == 1);
    BC_ASSERT(point(22, 10) == 1);
    BC_ASSERT(point(20, 9) == 1);
    BC_ASSERT(point(21, 9) == 0);
    xordrawb(20, 8, 3, 3);

    xordrawb(20, 8, 1, 3);
    BC_ASSERT(point(20, 9) == 1);
    xordrawb(20, 8, 1, 3);

    /* Clipping the left edge preserves the original dotted phase. */
    xorborder(-2, 8, 6, 5);
    BC_ASSERT(point(0, 8) == 0);
    BC_ASSERT(point(1, 8) == 1);
    BC_ASSERT(point(3, 8) == 0);
    BC_ASSERT(point(3, 9) == 1);
    BC_ASSERT(point(3, 10) == 0);
    xorborder(-2, 8, 6, 5);
    xorborder(20, 8, 1, 5);
    BC_ASSERT(point(20, 9) == 0);

    width = (int)bc_video_width();
    height = (int)bc_video_height();
    radius = height >> 4;

    /* SET: line, horizontal/vertical line, circle and ellipse. */
    bc_draw_line_op(8, 12, width >> 2, height >> 2, BC_DRAW_SET);
    bc_draw_hline_op(8, width >> 2, 12, BC_DRAW_SET);
    bc_draw_vline_op(8, 12, height >> 2, BC_DRAW_SET);
    bc_draw_circle_op(width >> 2, height >> 1, radius, BC_DRAW_SET);
    bc_draw_ellipse_op(3 * width >> 2, height >> 1, width >> 3, height >> 4, BC_DRAW_SET);

    /* Exercise XOR and CLEAR on line primitives too. */
    bc_draw_line_op(width >> 1, 8, width - 9, height >> 2, BC_DRAW_XOR);
    bc_draw_hline_op((width >> 1) - 8, (width >> 1) + 8, 12, BC_DRAW_CLEAR);

    bc_test_step_shot(tag);
}

int main(void) {
    if (bc_test_init() != BC_OK) {
        bc_test_finish();
        return 0;
    }
    BC_ASSERT(bc_select_screen(BC_SCREEN_0) == BC_OK);
    draw_mode(BC_VIDEO_ZX_256, "zx256");
    draw_mode(BC_VIDEO_ZX_MULTICOLOR, "multicolor");
    draw_mode(BC_VIDEO_PENTAGON_256, "pentagon256");
    draw_mode(BC_VIDEO_ATM_320, "atm320");
    draw_mode(BC_VIDEO_ATM_640, "atm640");
    bc_test_finish();
    return 0;
}
