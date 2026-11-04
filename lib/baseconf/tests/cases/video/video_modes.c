#include <baseconf.h>

#include "bc_test.h"

typedef struct {
    bc_video_mode_t mode;
    uint16_t width, height;
    uint8_t page0, page1;
    const char *name;
} mode_case_t;

static const mode_case_t modes[] = {
    {BC_VIDEO_ZX_256, 256, 192, 5, 5, "zx256"},
    {BC_VIDEO_ZX_MULTICOLOR, 256, 192, 5, 5, "multicolor"},
    {BC_VIDEO_PENTAGON_256, 256, 192, 4, 5, "pentagon256"},
    {BC_VIDEO_ATM_320, 320, 200, 1, 5, "atm320"},
    {BC_VIDEO_ATM_640, 640, 200, 1, 5, "atm640"},
    {BC_VIDEO_ATM_TEXT_80, 80, 25, 1, 5, "atm_text80"},
    {BC_VIDEO_EVO_TEXT_80, 80, 25, 8, 8, "evo_text80"},
};

static void check_mode(const mode_case_t *test) {
    bc_surface_t front = bc_screen_surface(test->mode, BC_SCREEN_0);
    bc_surface_t back = bc_screen_surface(test->mode, BC_SCREEN_1);
    BC_ASSERT(front.mode == test->mode && front.page0 == test->page0 && front.page1 == test->page1);
    BC_ASSERT(back.mode == test->mode && back.page0 == test->page0 + 2 &&
              back.page1 == test->page1 + 2);
    BC_ASSERT(bc_set_video_mode(test->mode) == BC_OK);
    BC_ASSERT(bc_get_video_mode() == test->mode);
    BC_ASSERT(bc_video_width() == test->width && bc_video_height() == test->height);
    BC_ASSERT(bc_select_draw_screen(BC_SCREEN_0) == BC_OK);
    BC_ASSERT(bc_set_video_mode(BC_VIDEO_UNKNOWN) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_get_video_mode() == test->mode);
    bc_test_step_marker(test->name);
}

int main(void) {
    uint8_t i;
    if (bc_test_init() == BC_OK) {
        BC_ASSERT(bc_select_screen(BC_SCREEN_0) == BC_OK);
        for (i = 0; i < sizeof(modes) / sizeof(modes[0]); ++i) check_mode(&modes[i]);
    }
    bc_test_finish();
    return 0;
}
