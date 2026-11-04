#include <baseconf.h>
#include <baseconf/graphics.h>
#include <baseconf/video.h>
#include <graphics.h>

#include "bc_test.h"

int main(void) {
    bc_surface_t surface;
    uint8_t index = 0;
    if (bc_test_init() == BC_OK) {
        BC_ASSERT(bc_select_screen(BC_SCREEN_0) == BC_OK);
        BC_ASSERT(bc_set_video_mode(BC_VIDEO_ATM_320) == BC_OK);
        surface = bc_screen_surface(BC_VIDEO_ATM_320, BC_SCREEN_0);
        clg();

        BC_ASSERT(bc_graphics_end() == BC_NOT_INITIALIZED);
        BC_ASSERT(bc_graphics_begin() == BC_OK);
        bc_draw_hline(32, 80, 48);
        BC_ASSERT(bc_graphics_begin() == BC_OK);
        bc_draw_vline(80, 48, 80);
        BC_ASSERT(bc_read_pixel_index(80, 64, &index) == BC_OK);
        BC_ASSERT(index == 7);
        BC_ASSERT(bc_select_draw_surface(&surface) == BC_BUSY);
        BC_ASSERT(bc_graphics_end() == BC_OK);

        /* Closing the inner group still leaves the outer group active. */
        BC_ASSERT(bc_select_draw_surface(&surface) == BC_BUSY);
        bc_draw_hline(32, 80, 80);
        BC_ASSERT(bc_graphics_end() == BC_OK);
        BC_ASSERT(bc_graphics_end() == BC_NOT_INITIALIZED);
        BC_ASSERT(bc_select_draw_surface(&surface) == BC_OK);
        bc_draw_vline(32, 48, 80);

        BC_ASSERT(point(56, 48) == 1);
        BC_ASSERT(point(80, 64) == 1);
        BC_ASSERT(point(56, 80) == 1);
        BC_ASSERT(point(32, 64) == 1);
        BC_ASSERT(point(56, 64) == 0);
        bc_test_step_shot("nested");
    }
    bc_test_finish();
    return 0;
}
