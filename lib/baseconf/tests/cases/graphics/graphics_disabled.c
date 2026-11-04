#include <baseconf.h>

#include "bc_test.h"

int main(void) {
    uint8_t index = 0xff;
    if (bc_test_init() == BC_OK) {
        BC_ASSERT(bc_select_screen(BC_SCREEN_0) == BC_OK);
        BC_ASSERT(bc_set_video_mode(BC_VIDEO_ZX_256) == BC_OK);
        BC_ASSERT(bc_graphics_begin() == BC_UNSUPPORTED);
        BC_ASSERT(bc_read_pixel_index(0, 0, &index) == BC_UNSUPPORTED);
        BC_ASSERT(index == 0xff);
        BC_ASSERT(bc_graphics_end() == BC_NOT_INITIALIZED);
        bc_test_step_marker("disabled");
    }
    bc_test_finish();
    return 0;
}
