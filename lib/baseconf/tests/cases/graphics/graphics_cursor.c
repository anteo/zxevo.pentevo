#include <baseconf.h>
#include <graphics.h>

#include "bc_test.h"

static uint8_t lines_match(void) {
    int x, y;
    for (y = 43; y <= 105; ++y) {
        for (x = 31; x <= 105; ++x) {
            int expected = (y == 44 && x >= 32 && x <= 80) || (x == 80 && y >= 44 && y <= 76) ||
                           (y == 76 && x >= 48 && x <= 80) || (y == 104 && x >= 60 && x <= 104) ||
                           (x == 104 && y >= 88 && y <= 104);
            if (point(x, y) != expected) return 0;
        }
    }
    return 1;
}

int main(void) {
    if (bc_test_init() == BC_OK) {
        BC_ASSERT(bc_select_screen(BC_SCREEN_0) == BC_OK);
        BC_ASSERT(bc_set_video_mode(BC_VIDEO_ZX_256) == BC_OK);
        clg();

        pen_down();
        setpos(32, 44);
        BC_ASSERT(getx() == 32 && gety() == 44);
        drawto(80, 44);
        BC_ASSERT(getx() == 80 && gety() == 44);
        drawr(0, 32);
        BC_ASSERT(getx() == 80 && gety() == 76);
        drawr(-32, 0);
        BC_ASSERT(getx() == 48 && gety() == 76);

        /* Both calls move the cursor with the pen up, leaving a gap. */
        pen_up();
        drawto(48, 104);
        BC_ASSERT(getx() == 48 && gety() == 104);
        drawr(12, 0);
        BC_ASSERT(getx() == 60 && gety() == 104);
        pen_down();
        drawto(104, 104);
        BC_ASSERT(getx() == 104 && gety() == 104);
        drawr(0, -16);
        BC_ASSERT(getx() == 104 && gety() == 88);

        BC_ASSERT(lines_match());
        bc_test_step_shot("cursor");
    }
    bc_test_finish();
    return 0;
}
