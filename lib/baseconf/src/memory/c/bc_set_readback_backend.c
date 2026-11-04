#include <baseconf.h>
#include "../internal.h"
#include "../../video/internal.h"

bc_status_t bc_set_readback_backend(bc_readback_backend_t backend) {
    if (backend > BC_READBACK_BE) return BC_INVALID_ARGUMENT;
    if (bc_mem_active_window_lease) return BC_BUSY;
    if (bc_video_operation_depth) return BC_BUSY;
    if (bc_mem_readback_backend == backend) return BC_OK;
    bc_mem_readback_backend = backend;
    if (bc_mem_initialized) {
        bc__mem_refresh_all_windows();
        bc__video_invalidate_cache();
    }
    return BC_OK;
}
