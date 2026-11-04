#ifndef BASECONF_VIDEO_H
#define BASECONF_VIDEO_H

#include <stdint.h>
#include <baseconf.h>

/* Decode the live FPGA mode registers; undocumented combinations return BC_VIDEO_UNKNOWN. */
bc_video_mode_t bc_get_video_mode(void);
/* Read the current beam color through 0DBD; it is raster-phase dependent. */
bc_status_t bc_read_beam_color(uint8_t *color);
/* Read the glyph-row byte currently exposed by port 0EBDh, not a row selected by
 * character code. The result depends on raster phase. NULL returns BC_INVALID_ARGUMENT. */
bc_status_t bc_read_font_row(uint8_t *row);
/* Read a pixel's palette index from the selected draw surface (not necessarily
 * the displayed screen). Returns BC_INVALID_ARGUMENT for NULL index or x/y
 * outside its dimensions, BC_UNSUPPORTED for text or excluded primitive modes,
 * and propagates video initialization/paging errors. */
bc_status_t bc_read_pixel_index(uint16_t x, uint16_t y, uint8_t *index);
/* Describe a standard hardware screen without changing paging or video state.
 * Supply a mode included in the library build and BC_SCREEN_0 or BC_SCREEN_1;
 * arguments are not validated. Single-page modes return page0 == page1. */
bc_surface_t bc_screen_surface(bc_video_mode_t mode, bc_screen_t screen);
/* Select a standard draw screen for the live hardware video mode. Its pages
 * must already be writable RAM in W1..W3 of the active map, with substitution
 * disabled; this call does not map them or change the displayed screen.
 * Requires bc_init; returns BC_UNAVAILABLE if pages are missing or the live
 * mode is unknown, BC_BUSY during a window lease/video operation, and
 * BC_INVALID_ARGUMENT for an invalid screen. */
bc_status_t bc_select_draw_screen(bc_screen_t screen);
/* Copy a software draw-surface descriptor and use its mode for drawing/geometry.
 * Does not set hardware mode, map RAM or select the displayed screen. The caller
 * prepares writable surface pages before rendering. ZX_256, ZX_MULTICOLOR and
 * EVO_TEXT_80 require equal pages; other modes require two different pages.
 * Returns BC_INVALID_ARGUMENT for NULL/invalid descriptors, BC_UNSUPPORTED for
 * excluded modes, and BC_BUSY during a video operation. */
bc_status_t bc_select_draw_surface(const bc_surface_t *surface);
/* Store the foreground color for subsequent console/text output and packed-pixel
 * drawing. Supply 0..15; no validation or palette write is performed. */
void bc_set_ink(uint8_t index);
/* Store the background color for subsequent console/text output, clears and
 * packed-pixel erase operations. Supply 0..15; no validation or palette write. */
void bc_set_paper(uint8_t index);
/* Set the hardware video mode and prepare the currently displayed screen for drawing.
 * Requires bc_init and an enabled page manager. Reuses writable page mappings and
 * replaces mappings in unpinned W1..W3 as needed, preferring W3. Pin every window
 * whose current mapping must be retained. Keeps the screen number and contents.
 * Returns BC_UNAVAILABLE if the page manager is off or too few windows are available,
 * BC_BUSY during a video or window lease, and BC_UNSAFE_CONTEXT on readback mismatch.
 * Unknown modes return BC_INVALID_ARGUMENT; excluded modes return BC_UNSUPPORTED. */
bc_status_t bc_set_video_mode(bc_video_mode_t mode);
/* Select the displayed hardware screen; does not change the draw target or copy/clear pixels. */
bc_status_t bc_show_screen(bc_screen_t screen);
/* Return the software-selected mode's height (text modes report 25 rows). */
uint16_t bc_video_height(void);
/* Return the software-selected mode's width in pixels (text modes report 80 columns). */
uint16_t bc_video_width(void);

#endif
