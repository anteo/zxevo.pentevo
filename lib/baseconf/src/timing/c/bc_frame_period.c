#include <baseconf.h>
#include <baseconf/timing.h>

bc_status_t bc_frame_period(uint32_t *tstates) {
    bc_video_output_t output;
    uint32_t period;
    bc_status_t status;
    if (!tstates) return BC_INVALID_ARGUMENT;
    status = bc_read_video_output(&output);
    if (status != BC_OK) return status;
    switch (output.raster) {
    case BC_RASTER_PENTAGON:
        period = 71680u;
        break;
    case BC_RASTER_60HZ:
        period = 58688u;
        break;
    case BC_RASTER_48K:
        period = 69888u;
        break;
    case BC_RASTER_128K:
        period = 70908u;
        break;
    default:
        return BC_UNAVAILABLE;
    }
    *tstates = period;
    return BC_OK;
}
