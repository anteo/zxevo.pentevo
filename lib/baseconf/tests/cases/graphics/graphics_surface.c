#include <baseconf.h>
#include <graphics.h>

#include "bc_test.h"

static void check_buffers(void) {
    bc_surface_t front = bc_screen_surface(BC_VIDEO_ZX_256, BC_SCREEN_0);
    bc_surface_t back = bc_screen_surface(BC_VIDEO_ZX_256, BC_SCREEN_1);
    uint8_t index = 0xff;

    BC_ASSERT(bc_map_ram_page(BC_WINDOW_2, back.page0) == BC_OK);
    BC_ASSERT(bc_map_ram_page(BC_WINDOW_3, front.page0) == BC_OK);

    BC_ASSERT(bc_select_draw_screen(BC_SCREEN_0) == BC_OK);
    bc_set_ink(6);
    clg();
    fillb(32, 48, 48, 32);

    BC_ASSERT(bc_select_draw_screen(BC_SCREEN_1) == BC_OK);
    bc_set_ink(5);
    clg();
    bc_draw_line(112, 48, 160, 80);
    bc_draw_line(160, 48, 112, 80);
    BC_ASSERT(BC_PAGING_SCREEN(bc_readback_register(0x0a)) == BC_SCREEN_0);
    BC_ASSERT(bc_read_pixel_index(136, 64, &index) == BC_OK && index == 5);
    BC_ASSERT(point(56, 64) == 0);
    bc_test_step_shot("front");

    BC_ASSERT(bc_select_draw_screen(BC_SCREEN_0) == BC_OK);
    BC_ASSERT(bc_show_screen(BC_SCREEN_1) == BC_OK);
    BC_ASSERT(BC_PAGING_SCREEN(bc_readback_register(0x0a)) == BC_SCREEN_1);
    BC_ASSERT(bc_read_pixel_index(56, 64, &index) == BC_OK && index == 6);
    BC_ASSERT(point(136, 64) == 0);
    bc_test_step_shot("back");

    BC_ASSERT(bc_show_screen(BC_SCREEN_0) == BC_OK);
    BC_ASSERT(bc_read_pixel_index(256, 0, &index) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_read_pixel_index(0, 192, &index) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_read_pixel_index(0, 0, 0) == BC_INVALID_ARGUMENT);
    BC_ASSERT(index == 6);
}

static void check_rejected_surfaces(void) {
    const bc_surface_t unmapped = {BC_VIDEO_ZX_256, 63, 63};
    uint8_t index = 0xff;

    BC_ASSERT(bc_select_draw_surface(&unmapped) == BC_OK);
    BC_ASSERT(bc_graphics_begin() == BC_UNAVAILABLE);
    BC_ASSERT(bc_read_pixel_index(0, 0, &index) == BC_UNAVAILABLE);
    BC_ASSERT(index == 0xff);
    BC_ASSERT(bc_graphics_end() == BC_NOT_INITIALIZED);

    BC_ASSERT(bc_set_video_mode(BC_VIDEO_EVO_TEXT_80) == BC_OK);
    BC_ASSERT(bc_graphics_begin() == BC_UNSUPPORTED);
    BC_ASSERT(bc_read_pixel_index(0, 0, &index) == BC_UNSUPPORTED);
    BC_ASSERT(index == 0xff);
    BC_ASSERT(bc_graphics_end() == BC_NOT_INITIALIZED);
    bc_test_step_marker("rejected");
}

int main(void) {
    if (bc_test_init() == BC_OK) {
        BC_ASSERT(bc_select_screen(BC_SCREEN_0) == BC_OK);
        BC_ASSERT(bc_set_video_mode(BC_VIDEO_ZX_256) == BC_OK);
        check_buffers();
        check_rejected_surfaces();
    }
    bc_test_finish();
    return 0;
}
