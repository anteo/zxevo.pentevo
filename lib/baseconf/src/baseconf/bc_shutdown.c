#include <baseconf.h>
#include "../internal.h"
#include "../memory/internal.h"
#include "../video/internal.h"

void bc_shutdown(void) {
    /* The void API cannot report BC_BUSY; leave state intact until leases end. */
    if (bc_mem_active_window_lease || bc_video_operation_depth) return;
    bc_mem_initialized = 0;
    bc_video_surface_selected = 0;
    bc__video_invalidate_cache();
}
