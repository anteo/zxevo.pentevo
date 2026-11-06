#include <arch/z80.h>
#include <baseconf.h>
#include <string.h>

#include "bc_test.h"

enum { FONT_BYTES = 2048, PORT_BF = 0x00bf };

static uint8_t *const source = (uint8_t *)0x8000u;
static uint8_t *const destination = (uint8_t *)0x8800u;
static uint8_t *const backup = (uint8_t *)0x9000u;

static const uint8_t diamond[8] = {0x18, 0x3c, 0x7e, 0xff, 0xff, 0x7e, 0x3c, 0x18};
static const uint8_t checker[8] = {0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55};

static uint8_t ram_was_restored(void) {
    uint16_t i;
    for (i = 0; i < FONT_BYTES; ++i) {
        uint8_t pattern = (uint8_t)(i ^ (i >> 8) ^ 0xa7u);
        if (destination[i] != pattern || backup[i] != pattern) return 0;
    }
    return 1;
}

int main(void) {
    uint16_t i;
    uint8_t bf;
    if (bc_test_init() == BC_OK) {
        BC_ASSERT(bc_select_screen(BC_SCREEN_0) == BC_OK);
        BC_ASSERT(bc_set_video_mode(BC_VIDEO_EVO_TEXT_80) == BC_OK);
        BC_ASSERT(bc_map_ram_page(BC_WINDOW_2, 48) == BC_OK);
        BC_ASSERT(bc_text_clear() == BC_OK);
        BC_ASSERT(bc_text_gotoxy(12, 8) == BC_OK);
        BC_ASSERT(bc_text_puts("ABAB") == BC_OK);

        bf = z80_inp(PORT_BF);
        memset(source, 0, FONT_BYTES);
        memcpy(source + 'A' * 8, diamond, sizeof(diamond));
        memcpy(source + 'B' * 8, checker, sizeof(checker));
        BC_ASSERT(bc_upload_font_to(source, destination + 1) == BC_INVALID_ARGUMENT);
        BC_ASSERT(bc_upload_font_to(source, source) == BC_INVALID_ARGUMENT);
        BC_ASSERT(bc_upload_font_to(source, destination) == BC_OK);
        BC_ASSERT(memcmp(source, destination, FONT_BYTES) == 0);
        BC_ASSERT(z80_inp(PORT_BF) == bf);
        BC_ASSERT(bc_read_font_row(0) == BC_INVALID_ARGUMENT);
        bc_test_step_shot("upload");

        memcpy(source + 'A' * 8, checker, sizeof(checker));
        memcpy(source + 'B' * 8, diamond, sizeof(diamond));
        for (i = 0; i < FONT_BYTES; ++i) destination[i] = (uint8_t)(i ^ (i >> 8) ^ 0xa7u);
        BC_ASSERT(bc_upload_font_preserving(source, destination, destination) ==
                  BC_INVALID_ARGUMENT);
        BC_ASSERT(bc_upload_font_preserving(source, destination, backup) == BC_OK);
        BC_ASSERT(ram_was_restored());
        BC_ASSERT(z80_inp(PORT_BF) == bf);
        bc_test_step_shot("preserving");
    }
    bc_test_finish();
    return 0;
}
