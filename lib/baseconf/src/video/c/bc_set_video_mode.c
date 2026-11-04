#include <baseconf.h>
#include "../internal.h"
#include "../../memory/internal.h"

/* Reuse writable mappings. Plan all replacements before touching the map. */
static bc_status_t plan_surface_windows(bc_map_t map, const bc_surface_t *surface,
                                        bc_window_t replacements[2]) {
    bc_window_t mapped[2];
    uint8_t reserved;
    uint8_t i;

    if (bc__video_find_page_windows(map, surface, BC_VIDEO_SKIP_WINDOW_ERRORS, mapped) == BC_OK) {
        replacements[0] = replacements[1] = BC_WINDOW_0;
        return BC_OK;
    }
    reserved = (uint8_t)(bc_mem_pinned_window_mask | (1u << mapped[0]) | (1u << mapped[1]));
    reserved |= (uint8_t)(1u << (bc__mem_read_sp() >> 14));
    for (i = 0; i < 2; ++i) {
        bc_window_t window;
        replacements[i] = BC_WINDOW_0;
        if (mapped[i] != BC_WINDOW_0 || (i && surface->page0 == surface->page1)) continue;

        for (window = BC_WINDOW_3; window != BC_WINDOW_0; --window)
            if (!(reserved & (1u << window))) break;
        if (window == BC_WINDOW_0) return BC_UNAVAILABLE;

        replacements[i] = window;
        reserved |= (uint8_t)(1u << window);
    }
    return BC_OK;
}

bc_status_t bc_set_video_mode(bc_video_mode_t mode) {
    bc_surface_t surface;
    bc_window_t replacements[2];
    bc_status_t status;
    bc_map_t map;
    uint8_t paging;
    uint8_t eff7;
    uint8_t sys;
    uint8_t video;

    if (mode > BC_VIDEO_EVO_TEXT_80) return BC_INVALID_ARGUMENT;
#ifndef BASECONF_VIDEO_ALL_MODES
    if (!bc__video_mode_enabled(mode)) return BC_UNSUPPORTED;
#endif
    if (!bc_mem_initialized) return BC_NOT_INITIALIZED;
    if (bc_mem_active_window_lease || bc_video_operation_depth) return BC_BUSY;

    sys = bc_readback_register(0x0c);
    if (!(sys & 0x20u)) return BC_UNAVAILABLE;
    paging = bc_readback_register(0x0a);
    map = BC_PAGING_ACTIVE_MAP(paging);
    if (map != (bc_map_t)bc_mem_active_map) return BC_UNSAFE_CONTEXT;
    surface = bc_screen_surface(mode, BC_PAGING_SCREEN(paging));
    status = plan_surface_windows(map, &surface, replacements);
    if (status != BC_OK) return status;

    eff7 = bc_readback_register(0x0b);
    switch (mode) {
#if defined(BASECONF_VIDEO_MODE_ENABLED_ZX_256)
    case BC_VIDEO_ZX_256:
        video = 3;
        eff7 &= (uint8_t)~0x21u;
        break;
#endif
#if defined(BASECONF_VIDEO_MODE_ENABLED_ZX_MULTICOLOR)
    case BC_VIDEO_ZX_MULTICOLOR:
        video = 3;
        eff7 = (uint8_t)((eff7 & (uint8_t)~0x01u) | 0x20u);
        break;
#endif
#if defined(BASECONF_VIDEO_MODE_ENABLED_PENTAGON_256)
    case BC_VIDEO_PENTAGON_256:
        video = 3;
        eff7 = (uint8_t)((eff7 & (uint8_t)~0x20u) | 0x01u);
        break;
#endif
#if defined(BASECONF_VIDEO_MODE_ENABLED_ATM_320)
    case BC_VIDEO_ATM_320:
        video = 0;
        eff7 &= (uint8_t)~0x21u;
        break;
#endif
#if defined(BASECONF_VIDEO_MODE_ENABLED_ATM_640)
    case BC_VIDEO_ATM_640:
        video = 2;
        eff7 &= (uint8_t)~0x21u;
        break;
#endif
#if defined(BASECONF_VIDEO_MODE_ENABLED_ATM_TEXT_80)
    case BC_VIDEO_ATM_TEXT_80:
        video = 6;
        eff7 &= (uint8_t)~0x21u;
        break;
#endif
    default:
#if defined(BASECONF_VIDEO_MODE_ENABLED_EVO_TEXT_80)
        video = 7;
#else
        return BC_UNSUPPORTED;
#endif
        eff7 &= (uint8_t)~0x21u;
        break;
    }
    /* A write can change the mode even if the following readback fails. */
    bc__video_invalidate_cache();
    status = bc__mem_write_eff7(eff7);
    if (status != BC_OK) return status;
    {
        uint8_t bf = bc__mem_shadow_scope_begin();
        z80_outp(bc__video_system_port(0), (uint8_t)((sys & 0x08u) | video));
        bc__mem_shadow_scope_end(bf);
    }
    if (bc_get_video_mode() != mode) return BC_UNSAFE_CONTEXT;

    if (replacements[0] || replacements[1]) {
        bc_window_config_t config = {BC_RAM, surface.page0, 0, 0};
        if (replacements[0]) {
            bc__mem_apply_window(replacements[0], &config);
        }
        if (replacements[1]) {
            config.physical_page = surface.page1;
            bc__mem_apply_window(replacements[1], &config);
        }
    }
    return bc__video_commit_surface(&surface);
}
