#include <baseconf/font.h>

#ifdef BASECONF_CONSOLE_NO_DEFAULT_FONT
const bc_font_t *bc_console_active_font;
#else
const bc_font_t *bc_console_active_font = &baseconf_font8;
#endif
