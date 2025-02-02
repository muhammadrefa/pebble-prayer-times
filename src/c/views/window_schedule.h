#pragma once

#include <pebble.h>

int schedule_window_set_prayer_time(int idx, tm *_time);
int schedule_window_set_date(tm *_time);
void schedule_window_set_coord(double lat, double lng);
void schedule_window_push();
