#pragma once

#include <pebble.h>

void main_window_init(void (*callback_schedule)(), void(*callback_settings)());
void main_window_push();
