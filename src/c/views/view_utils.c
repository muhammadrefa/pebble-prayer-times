#include <pebble.h>
#include "view_utils.h"

const char *view_util_time_name[] = {"Fajr", "Sunrise", "Dhuhr", "Asr", "Sunset", "Maghrib", "Isha"};

tm view_util_apply_offset(tm *original, int offset)
{
    tm offset_time = *original;
    offset_time.tm_min += offset;
    if (offset_time.tm_min >= 60)
    {
        offset_time.tm_hour += 1;
        offset_time.tm_min -= 60;
    }
    else if (offset_time.tm_min < 0)
    {
        offset_time.tm_hour -= 1;
        offset_time.tm_min += 60;
    }

    return offset_time;
}