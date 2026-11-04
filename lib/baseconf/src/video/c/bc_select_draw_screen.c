#include <baseconf.h>
#include "../internal.h"
#include "../../memory/internal.h"

bc_status_t bc_select_draw_screen(bc_screen_t screen) {
    bc_video_mode_t mode;
    bc_surface_t surface;
    bc_map_t map;
    bc_window_t windows[2];
    bc_status_t status;

    if (!bc_mem_initialized) return BC_NOT_INITIALIZED;
    if (bc_mem_active_window_lease) return BC_BUSY;
    if (screen > BC_SCREEN_1) return BC_INVALID_ARGUMENT;
    mode = bc_get_video_mode();
    if (mode == BC_VIDEO_UNKNOWN) return BC_UNAVAILABLE;
#ifndef BASECONF_VIDEO_ALL_MODES
    if (!bc__video_mode_enabled(mode)) return BC_UNSUPPORTED;
#endif
    surface = bc_screen_surface(mode, screen);
    map = (bc_map_t)((bc_readback_register(0x0a) >> 4) & 1u);
    status = bc__video_find_page_windows(map, &surface, BC_VIDEO_STOP_ON_WINDOW_ERROR, windows);
    if (status != BC_OK) return status;
    return bc_select_draw_surface(&surface);
}
