#include <baseconf.h>
#include <baseconf/console.h>
#include <baseconf/font.h>
#include <conio.h>
#include <intrinsic.h>
#include <stdio.h>

#include "bc_test.h"

static const char wrap_text[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz"
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz";

static void report_counts(const char *tag, int format_count, int wrap_count) {
    bc_test_output_begin();
    bc_test_output(tag);
    bc_test_output(" format=");
    bc_test_output_u16((uint16_t)format_count);
    bc_test_output(" wrap=");
    bc_test_output_u16((uint16_t)wrap_count);
    bc_test_output_char('\n');
    bc_test_step_text("counts");
}

static void draw_graphics_case(bc_video_mode_t mode, const bc_font_t *font, const char *tag,
                               uint8_t text_rows) {
    int format_count, wrap_count;
    uint8_t rows;
    if (bc_set_video_mode(mode) != BC_OK) {
        bc_test_step_skip(tag, "video mode unavailable");
        bc_test_step_skip("counts", "video mode unavailable");
        return;
    }
    BC_ASSERT(bc_console_set_font(font) == BC_OK);
    textcolor(10);
    textbackground(1);
    clrscr();
    format_count = printf("%d|%u|%04X|%-4s|%c|%%\n", -42, 65535u, 42u, "ok", 'Z');
    BC_ASSERT(format_count == 24);
    printf("zero:%05d left:%-5u\n", -7, 9u);
    gotoxy(2, 3);
    printf("cursor at 2,3");
    gotoxy(0, 6);
    puts("puts path");
    gotoxy(0, 7);
    (void)putchar('!');
    gotoxy(0, 8);
    wrap_count = printf("wrap:%s", wrap_text);
    BC_ASSERT(wrap_count == (int)(5u + sizeof(wrap_text) - 1u));
    rows = text_rows ? 25u : (uint8_t)(bc_video_height() / 8u);
    gotoxy(0, (unsigned int)(rows - 1u));
    printf("bottom row\nscroll one\nscroll two\n");
    gotoxy(0, 0);
    (void)printf("%d|%u|%04X|%-4s|%c|%%\n", -42, 65535u, 42u, "ok", 'Z');
    bc_test_step_shot(tag);
    report_counts(tag, format_count, wrap_count);
}

static void draw_text_case(bc_video_mode_t mode, const char *tag) {
    int format_count, wrap_count;
    if (bc_set_video_mode(mode) != BC_OK) {
        bc_test_step_skip(tag, "video mode unavailable");
        bc_test_step_skip("counts", "video mode unavailable");
        return;
    }
    textcolor(14);
    textbackground(1);
    clrscr();
    format_count = printf("%d|%u|%04X|%-4s|%c|%%\n", -42, 65535u, 42u, "ok", 'Z');
    BC_ASSERT(format_count == 24);
    printf("zero:%05d left:%-5u\n", -7, 9u);
    gotoxy(0, 7);
    (void)putchar('!');
    gotoxy(0, 8);
    wrap_count = printf("wrap:%s", wrap_text);
    BC_ASSERT(wrap_count == (int)(5u + sizeof(wrap_text) - 1u));
    gotoxy(0, 24);
    printf("bottom row\nscroll one\nscroll two\n");
    gotoxy(0, 0);
    (void)printf("%d|%u|%04X|%-4s|%c|%%\n", -42, 65535u, 42u, "ok", 'Z');
    bc_test_step_shot(tag);
    report_counts(tag, format_count, wrap_count);
}

int main(void) {
    intrinsic_di();
    if (bc_test_init() != BC_OK) {
        bc_test_finish();
        return 0;
    }
    BC_ASSERT(bc_select_screen(BC_SCREEN_0) == BC_OK);
    draw_graphics_case(BC_VIDEO_ZX_256, &baseconf_font5, "zx256_font5", 0);
    draw_graphics_case(BC_VIDEO_ATM_640, &baseconf_font5, "atm640_font5", 0);
    draw_graphics_case(BC_VIDEO_ATM_320, &baseconf_font5, "atm320_font5", 0);
    draw_text_case(BC_VIDEO_ATM_TEXT_80, "atm_text80");
    draw_text_case(BC_VIDEO_EVO_TEXT_80, "evo_text80");
    bc_test_finish();
    return 0;
}
