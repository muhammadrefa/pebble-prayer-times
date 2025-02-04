#pragma once

#include <pebble.h>

// int schedule_window_set_prayer_time(int idx, tm *_time);
void schedule_window_set_prayer_times(tm *_time, int8_t* time_offset);
int schedule_window_set_date(tm *_time);
void schedule_window_set_coord(double lat, double lng);
void schedule_window_push();
void schedule_window_init();
void schedule_window_set_selected_cb(void (*callback_selected)());
