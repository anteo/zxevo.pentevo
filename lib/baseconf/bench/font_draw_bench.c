#include <baseconf.h>
#include <intrinsic.h>
#include "../src/video/internal.h"

#define BENCH_GLYPHS 64u
#ifndef BENCH_MODE
#define BENCH_MODE 0
#endif

void font5_bench_start(void) {
    __asm nop __endasm;
}
void font5_bench_done(void) {
    __asm nop __endasm;
}
#if BENCH_MODE != 4
void font8_bench_start(void) {
    __asm nop __endasm;
}
void font8_bench_done(void) {
    __asm nop __endasm;
}
#endif

static void draw_repeated(const bc_font_t *font) {
    uint8_t i;
    for (i = 0; i < BENCH_GLYPHS; ++i) font->draw((uint16_t)((i & 3u) * 6u), 0, font, 'p', 7, 0);
}

int main(void) {
#if BENCH_MODE == 4
    const uint8_t pinned = 0x06;
#else
    const uint8_t pinned = 0x07;
#endif
    intrinsic_di();
    bc_ers_set_profile(BC_ERS_NONE);
    bc_set_pinned_windows(pinned);
    if (bc_init() != BC_OK) return 1;
#if BENCH_MODE == 4
    if (bc_set_video_mode(BC_VIDEO_ATM_640) != BC_OK) return 2;
    if (bc_map_ram_page(BC_WINDOW_3, 1) != BC_OK) return 3;
    {
        const bc_surface_t surface = {BC_VIDEO_ATM_640, 1, 5};
        if (bc_select_draw_surface(&surface) != BC_OK) return 4;
    }
#else
    if (bc_set_video_mode(BC_VIDEO_ZX_256) != BC_OK) return 2;
    if (bc_show_screen(BC_SCREEN_0) != BC_OK) return 3;
    if (bc_select_draw_screen(BC_SCREEN_0) != BC_OK) return 4;
#endif
    if (bc_console_set_font(&baseconf_font5) != BC_OK) return 5;
    if (bc__video_begin() != BC_OK) return 6;
    font5_bench_start();
    draw_repeated(&baseconf_font5);
    font5_bench_done();
#if BENCH_MODE != 4
    (void)bc__video_end();
    if (bc_console_set_font(&baseconf_font8) != BC_OK) return 7;
    if (bc__video_begin() != BC_OK) return 8;
    font8_bench_start();
    draw_repeated(&baseconf_font8);
    font8_bench_done();
    (void)bc__video_end();
#endif
    for (;;) {
    }
}
