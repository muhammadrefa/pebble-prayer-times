#pragma once

#include <pebble.h>

extern const char *view_util_time_name[];

tm view_util_apply_offset(tm *original, int offset);
