#ifndef BASECONF_TEXT_H
#define BASECONF_TEXT_H

#include <baseconf.h>

/* clear/gotoxy/putc/puts operate on the selected ATM_TEXT_80 or EVO_TEXT_80
 * surface (80x25); draw/draw_font render bitmap glyphs in pixel graphics modes.
 * Cursor-based calls share the console cursor. All use the current text colors
 * and need no z88dk stdio or console shadow buffer.
 * Functions return BC_OK on success; rendering requires bc_init and writable
 * surface pages already mapped, and propagates video-operation errors. */

/* Fill the hardware-text surface with spaces and the current text attribute;
   reset the cursor to (0,0). Return BC_UNSUPPORTED in pixel graphics modes. */
bc_status_t bc_text_clear(void);

/* Draw a NUL-terminated string in pixel graphics modes using the font selected
   by bc_console_set_font: x = column * font->advance, y = row * 8.
   Stop at the right edge without wrapping, scrolling or changing the cursor.
   Bytes outside ASCII 32..127 become '?'; control codes have no special effect.
   Return BC_INVALID_ARGUMENT for NULL text, no selected font or a starting cell
   outside the width/advance by height/8 grid; BC_UNSUPPORTED in text modes. */
bc_status_t bc_text_draw(uint8_t column, uint8_t row, const char *text);

/* Draw a NUL-terminated string with a raw table of 96 glyphs (ASCII 32..127),
   eight bytes per glyph. Always use an 8x8 cell and eight-pixel advance,
   independently of the selected console font; the selected font is unchanged.
   Clipping, character normalization and cursor behavior follow bc_text_draw.
   Return BC_INVALID_ARGUMENT for NULL text/font or an out-of-grid starting
   cell; BC_UNSUPPORTED in hardware text modes. */
bc_status_t bc_text_draw_font(uint8_t column, uint8_t row, const char *text, const uint8_t *font);

/* Set the shared hardware-text cursor to zero-based column/row without drawing.
   Valid coordinates are column 0..79 and row 0..24; otherwise return
   BC_INVALID_ARGUMENT. Return BC_UNSUPPORTED in pixel graphics modes. */
bc_status_t bc_text_gotoxy(uint8_t column, uint8_t row);

/* Write one byte at the hardware-text cursor using the current text attribute.
   '\n' moves to column zero of the next row, '\r' to column zero, '\b' moves
   left without erasing (stopping at column zero), and '\t' writes spaces to
   the next multiple-of-eight column. Ignore other bytes below 32.
   Wrap at column 80 without scrolling; at row 24, further newlines/wraps stay
   on that row and subsequent output overwrites it.
   Return BC_UNSUPPORTED in pixel graphics modes. */
bc_status_t bc_text_putc(char character);

/* Write a NUL-terminated string at the hardware-text cursor with the same
   attributes, control codes and wrapping as bc_text_putc, using one video
   operation for the whole string. Return BC_INVALID_ARGUMENT for NULL text;
   BC_UNSUPPORTED in pixel graphics modes. */
bc_status_t bc_text_puts(const char *text);

#endif
