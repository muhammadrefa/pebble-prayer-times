#pragma once

#include <pebble.h>

#define PIN_WINDOW_SIZE GSize(128, 34)

void offset_window_init(int _prayer_idx, tm* _prayer_time, int8_t* _current_offset, void (*callback_store)(int, int));
void offset_window_push();
