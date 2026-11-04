#ifndef BASECONF_FONT_H
#define BASECONF_FONT_H

#include <stdint.h>
typedef struct bc_font bc_font_t;

#include <baseconf.h>

/* Optional bitmap-console font: 96 glyphs (codes 32..127), eight rows each. */
extern const uint8_t baseconf_font5_data[96][8];

/* Bitmap-font callback used by console/text rendering during an active video
 * operation. x/y are pixel coordinates; code is a character byte, and ink/paper
 * are palette indices. The callback does not return a rendering status.
 * Formatted output may draw a run with one font and color snapshot and update
 * the console cursor after the run. Draw using the supplied coordinates and
 * colors, and leave the console state alone. */
typedef void (*bc_font_draw_fn)(uint16_t x, uint16_t y, const bc_font_t *font, uint8_t code,
                                uint8_t ink, uint8_t paper);

#define BC_FONT_TRANSPARENT 0x01u
#define BC_FONT_OPAQUE 0x02u

/* Bitmap descriptor: built-in renderers use 96 glyphs for codes 32..127,
 * eight bytes per glyph, top row first and leftmost pixel in bit 7. width and
 * height are 1..8; advance is the horizontal step and must be >= width.
 * Console rows always advance by eight pixels, regardless of glyph height.
 * Drawing flags are interpreted by the callback. */
struct bc_font {
    const uint8_t *glyphs;
    uint8_t width;
    uint8_t height;
    uint8_t advance;
    uint8_t flags;
    bc_font_draw_fn draw;
};

/* Opaque 8x8 CRT font, advance 8; its renderer draws complete 8x8 cells. */
extern const bc_font_t baseconf_font8;
/* Opaque 5x7 font, advance 6; glyph data is baseconf_font5_data. */
extern const bc_font_t baseconf_font5;

/* Upload a raw 2 KiB hardware font as bc_upload_font_to does, but save and
 * restore the RAM bytes at destination. The uploaded hardware font remains.
 * destination has the same W1..W3/0800h alignment requirements as below.
 * backup is a separate 2 KiB writable RAM region in W0..W3; no alignment is needed.
 * All three CPU ranges must be non-null, fit below 10000h and be disjoint.
 * The caller guarantees writable RAM for backup, no physical aliases between
 * the three ranges, and no code, stack or data needed by this call in either
 * written buffer. Do not use selected draw-surface pages for either buffer.
 * Mappings remain stable and NMI is excluded throughout the call.
 * INT is masked around backup/upload/restore and its prior state is restored.
 * BF.2 is cleared before destination is restored; backup retains the saved bytes.
 * Returns BC_INVALID_ARGUMENT for invalid CPU ranges/alignment, BC_BUSY if BF.2
 * is already set, otherwise BC_OK. Physical mappings are not inspected. */
bc_status_t bc_upload_font_preserving(const uint8_t font[2048], uint8_t *destination,
                                      uint8_t backup[2048]);
/* Upload raw 2 KiB font data through BF.2 using the current CPU memory map.
 * font is readable; destination is disposable writable RAM in W1..W3, aligned
 * to 0800h. Both CPU ranges must be non-null, fit below 10000h and be disjoint.
 * The caller guarantees valid physical mappings, no physical overlap between
 * source/destination, and no code, stack or data needed by this call in destination.
 * Do not use selected draw-surface pages for destination. Keep mappings stable
 * and exclude NMI throughout the call: BF.2 mirrors every CPU memory write into
 * font RAM. INT is masked during upload and its prior state is restored.
 * Returns BC_INVALID_ARGUMENT for invalid CPU ranges/alignment, BC_BUSY if BF.2
 * is already set, otherwise BC_OK. Physical mappings are not inspected and
 * library initialization is not required. */
bc_status_t bc_upload_font_to(const uint8_t font[2048], uint8_t *destination);

#endif
