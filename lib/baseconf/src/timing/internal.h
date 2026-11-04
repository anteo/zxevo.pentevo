#ifndef BASECONF_TIMING_INTERNAL_H
#define BASECONF_TIMING_INTERNAL_H

#include <baseconf.h>
#include <baseconf/timing.h>
#include "../internal.h"

bc_status_t bc_set_cpu_speed(bc_cpu_speed_t speed);
bc_status_t bc_get_cpu_speed(bc_cpu_speed_t *speed);

#endif
