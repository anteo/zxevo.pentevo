#ifndef BASECONF_CONSOLE_H
#define BASECONF_CONSOLE_H

#include <baseconf.h>
#include <baseconf/font.h>

/* Clear the selected draw surface with the current paper/text attribute and
 * reset the cursor to (0,0) on success. Returns video-operation errors when
 * the surface cannot be accessed. */
bc_status_t bc_console_clear(void);
/* Return the selected bitmap font descriptor, or NULL if none is selected.
 * The returned pointer is borrowed; hardware-text output does not use it. */
const bc_font_t *bc_console_get_font(void);
/* Move the cursor without drawing. Coordinates are zero-based and must fit
 * the current grid: 80x25 in hardware text, width/font advance by height/8 in
 * graphics modes. Returns BC_INVALID_ARGUMENT for an out-of-grid position. */
bc_status_t bc_console_gotoxy(uint8_t column, uint8_t row);
/* Write at the cursor with the current colors and selected bitmap font, or
 * the hardware font in text modes. '\n' starts the next row, '\r' starts the
 * current row, '\b' moves left without erasing, and '\t' emits spaces to the
 * next multiple-of-eight column. Other bytes below 32 are ignored.
 * Wrap at the right edge; at the bottom row, newlines/wraps stay on that row
 * and subsequent output overwrites it. Does not scroll.
 * Returns BC_NOT_INITIALIZED if graphics output has no font; drawing also
 * propagates video-operation errors. */
bc_status_t bc_console_putc(char c);
/* Select a bitmap font, update the grid and reset the cursor to (0,0).
 * Requires non-NULL font, glyphs and draw callback, width/height 1..8, and
 * advance >= width; invalid descriptors return BC_INVALID_ARGUMENT.
 * Returns BC_UNSUPPORTED in hardware text modes. The descriptor is not copied:
 * keep it and its glyph data CPU-accessible while selected, and select it again
 * after changing advance so cursor geometry is updated. */
bc_status_t bc_console_set_font(const bc_font_t *font);

#endif
