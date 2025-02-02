#pragma once

#include <pebble.h>
#include "PrayTimes/prayertimes.h"

void pt_init(PrayerTimes_calculation_method method, PrayerTimes_juristic_method asr_method, PrayerTimes_adjusting_method hi_lat_adjusment);
void pt_deinit();
void pt_set_location(double latitude, double longitude);
void pt_set_timezone(double timezone);
void pt_get_schedules0(tm *time, double* times);
void pt_get_schedules(tm *time, tm* schedule);
void pt_time2str(char *time_str, bool is_24h, double time);
