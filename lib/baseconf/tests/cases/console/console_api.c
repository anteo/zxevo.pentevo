#include <baseconf.h>
#include <baseconf/console.h>
#include <baseconf/font.h>
#include <baseconf/text.h>

#include "bc_test.h"

static void test_graphics_console(void) {
    BC_ASSERT(bc_set_video_mode(BC_VIDEO_ZX_256) == BC_OK);
    BC_ASSERT(bc_console_get_font() == NULL);
    BC_ASSERT(bc_console_putc('A') == BC_NOT_INITIALIZED);
    BC_ASSERT(bc_console_set_font(NULL) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_console_set_font(&baseconf_font5) == BC_OK);
    BC_ASSERT(bc_console_get_font() == &baseconf_font5);
    BC_ASSERT(bc_console_clear() == BC_OK);
    BC_ASSERT(bc_console_gotoxy(41, 23) == BC_OK);
    BC_ASSERT(bc_console_gotoxy(42, 23) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_console_gotoxy(0, 24) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_console_putc('A') == BC_OK);
    BC_ASSERT(bc_console_putc('\n') == BC_OK);
    BC_ASSERT(bc_console_putc('\r') == BC_OK);
    BC_ASSERT(bc_console_putc('\b') == BC_OK);
    BC_ASSERT(bc_console_putc('\t') == BC_OK);

    BC_ASSERT(bc_text_clear() == BC_UNSUPPORTED);
    BC_ASSERT(bc_text_gotoxy(0, 0) == BC_UNSUPPORTED);
    BC_ASSERT(bc_text_putc('A') == BC_UNSUPPORTED);
    BC_ASSERT(bc_text_puts("text") == BC_UNSUPPORTED);
    BC_ASSERT(bc_text_puts(NULL) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_text_draw(0, 0, "draw") == BC_OK);
    BC_ASSERT(bc_text_draw(0, 0, NULL) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_text_draw(41, 0, "clip") == BC_OK);
    BC_ASSERT(bc_text_draw(42, 0, "edge") == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_text_draw_font(0, 1, "raw", &baseconf_font5_data[0][0]) == BC_OK);
    BC_ASSERT(bc_text_draw_font(31, 1, "clip", &baseconf_font5_data[0][0]) == BC_OK);
    BC_ASSERT(bc_text_draw_font(0, 1, NULL, &baseconf_font5_data[0][0]) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_text_draw_font(0, 1, "raw", NULL) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_text_draw_font(32, 0, "edge", &baseconf_font5_data[0][0]) == BC_INVALID_ARGUMENT);

    bc_test_step_marker("graphics_api");
}

static void test_hardware_text(void) {
    BC_ASSERT(bc_set_video_mode(BC_VIDEO_ATM_TEXT_80) == BC_OK);

    BC_ASSERT(bc_console_set_font(&baseconf_font5) == BC_UNSUPPORTED);
    BC_ASSERT(bc_console_get_font() == &baseconf_font5);
    BC_ASSERT(bc_console_clear() == BC_OK);
    BC_ASSERT(bc_console_gotoxy(79, 24) == BC_OK);
    BC_ASSERT(bc_console_gotoxy(80, 24) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_console_gotoxy(0, 25) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_console_putc('Z') == BC_OK);
    BC_ASSERT(bc_text_clear() == BC_OK);
    BC_ASSERT(bc_text_gotoxy(79, 24) == BC_OK);
    BC_ASSERT(bc_text_gotoxy(80, 24) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_text_gotoxy(0, 25) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_text_putc('A') == BC_OK);
    BC_ASSERT(bc_text_putc('\n') == BC_OK);
    BC_ASSERT(bc_text_puts("A\tB\rC\bD\ntext mode") == BC_OK);
    BC_ASSERT(bc_text_puts(NULL) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_text_draw(0, 0, "unsupported") == BC_UNSUPPORTED);
    BC_ASSERT(bc_text_draw_font(0, 0, "unsupported", &baseconf_font5_data[0][0]) == BC_UNSUPPORTED);

    bc_test_step_marker("text_api");
}

int main(void) {
    if (bc_test_init() != BC_OK) {
        BC_ASSERT(0);
        bc_test_finish();
        return 0;
    }

    BC_ASSERT(bc_select_screen(BC_SCREEN_0) == BC_OK);
    test_graphics_console();
    test_hardware_text();
    bc_test_finish();
    return 0;
}
