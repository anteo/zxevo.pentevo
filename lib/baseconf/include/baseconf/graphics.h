#ifndef BASECONF_GRAPHICS_H
#define BASECONF_GRAPHICS_H
#include <stdint.h>
#include <baseconf.h>

/* In ZX and ATM_640 bitmap modes, SET/CLEAR/XOR set/clear/toggle bitmap bits
 * without changing attributes. In packed-color modes, SET writes current ink,
 * CLEAR writes current paper, and XOR combines the pixel index with ink. */
typedef enum { BC_DRAW_SET = 0, BC_DRAW_CLEAR = 1, BC_DRAW_XOR = 2 } bc_draw_operation_t;

/* Begin a group of drawing calls using the selected draw surface. Requires
 * bc_init, an enabled page manager and surface pages already mapped writable.
 * Calls nest; pair each successful begin with one end. Returns BC_UNSUPPORTED
 * for text/excluded primitive modes, and propagates video/paging errors.
 * Do not change hardware mode or paging while a drawing group is active. */
bc_status_t bc_graphics_begin(void);
/* End one drawing group; returns BC_NOT_INITIALIZED if none is active.
 * The final end releases the drawing context without changing mappings. */
bc_status_t bc_graphics_end(void);

/* All primitives use zero-based pixel coordinates on the selected draw surface,
 * clipping pixels outside its bounds. They begin/end their own drawing group,
 * so an explicit bc_graphics_begin is optional. Unsupported modes, inaccessible
 * surfaces and unknown operations draw nothing; these void calls report no errors.
 * Available modes depend on the library's primitive-backend build settings. */

/* Draw a one-pixel circle outline centered at (x,y). Negative radius draws nothing. */
void bc_draw_circle_op(int x, int y, int radius, bc_draw_operation_t operation);
/* Draw an axis-aligned ellipse outline centered at (cx,cy), with semi-axes rx/ry.
 * Negative semi-axes draw nothing; a zero semi-axis produces a line segment. */
void bc_draw_ellipse_op(int cx, int cy, int rx, int ry, bc_draw_operation_t operation);
/* Draw an inclusive horizontal segment at y; endpoints may be in either order. */
void bc_draw_hline_op(int x0, int x1, int y, bc_draw_operation_t operation);
/* Draw a segment including both endpoints. */
void bc_draw_line_op(int x0, int y0, int x1, int y1, bc_draw_operation_t operation);
/* Draw an inclusive vertical segment at x; endpoints may be in either order. */
void bc_draw_vline_op(int x, int y0, int y1, bc_draw_operation_t operation);

/* Shorthands for the corresponding _op functions with BC_DRAW_SET. */
void bc_draw_circle(int x, int y, int radius);
void bc_draw_ellipse(int cx, int cy, int rx, int ry);
void bc_draw_hline(int x0, int x1, int y);
void bc_draw_line(int x0, int y0, int x1, int y1);
void bc_draw_vline(int x, int y0, int y1);

/* Shorthands for the corresponding _op functions with BC_DRAW_XOR. */
void bc_draw_circle_xor(int x, int y, int radius);
void bc_draw_ellipse_xor(int cx, int cy, int rx, int ry);
void bc_draw_hline_xor(int x0, int x1, int y);
void bc_draw_line_xor(int x0, int y0, int x1, int y1);
void bc_draw_vline_xor(int x, int y0, int y1);

/* Shorthands for the corresponding _op functions with BC_DRAW_CLEAR. */
void bc_draw_circle_clear(int x, int y, int radius);
void bc_draw_ellipse_clear(int cx, int cy, int rx, int ry);
void bc_draw_hline_clear(int x0, int x1, int y);
void bc_draw_line_clear(int x0, int y0, int x1, int y1);
void bc_draw_vline_clear(int x, int y0, int y1);

#endif
