#include <baseconf.h>
#include "../internal.h"
#include "../../internal.h"
#include "../../video/internal.h"

bc_status_t bc_resync(void) {
    bc_status_t status;
    if (!bc_mem_initialized) return BC_NOT_INITIALIZED;
    if (bc_mem_active_window_lease || bc_video_operation_depth) return BC_BUSY;

    bc__mem_refresh_all_windows();
    bc__video_invalidate_cache();
    if (!bc_video_surface_selected) return BC_OK;
    status = bc__video_sync_cache();
    return status;
}
