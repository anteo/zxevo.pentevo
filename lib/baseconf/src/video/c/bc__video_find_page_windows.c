#include "../internal.h"

bc_status_t bc__video_find_page_windows(bc_map_t map, const bc_surface_t *surface,
                                        uint8_t stop_on_error, bc_window_t windows[2]) {
    bc_window_config_t config;
    bc_window_t window;

    windows[0] = BC_WINDOW_0;
    windows[1] = BC_WINDOW_0;
    for (window = BC_WINDOW_1; window <= BC_WINDOW_3; ++window) {
        bc_status_t status = bc_get_window_config(map, window, &config);
        if (status != BC_OK) {
            if (stop_on_error) return status;
            continue;
        }
        if (config.kind != BC_RAM || config.substitute_from_7ffd_or_dos || config.write_protected)
            continue;
        if (windows[0] == BC_WINDOW_0 && config.physical_page == surface->page0)
            windows[0] = window;
        if (windows[1] == BC_WINDOW_0 && config.physical_page == surface->page1)
            windows[1] = window;
    }

    return (windows[0] && windows[1]) ? BC_OK : BC_UNAVAILABLE;
}
