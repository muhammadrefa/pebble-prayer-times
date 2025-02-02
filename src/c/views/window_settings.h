#pragma once

#include <pebble.h>

// void main_window_init(void (*callback_schedule)(), void(*callback_settings)());
void settings_window_init(void (*callback_changed)(), PTConfig *pt_cfg, Coordinate *coordinate);
void settings_window_push();
