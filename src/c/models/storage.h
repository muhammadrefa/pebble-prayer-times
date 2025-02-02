#pragma once

#include <stdint.h>
#include <pebble.h>

typedef struct _PTConfig
{
    uint8_t calc_method;
    uint8_t juristic;
    uint8_t adjustment;
    uint8_t use_offset;
} PTConfig;

typedef struct _Coordinate
{
    double latitude;
    double longitude;
} Coordinate;

typedef enum _StorageKey
{
    STORAGE_VERSION,
	STORAGE_PT_CONFIG,
    STORAGE_COORDINATE,
	STORAGE_TIME_OFFSET,
} StorageKey;

bool storage_load_pt_config(PTConfig *settings);
bool storage_store_pt_config(PTConfig *settings);
bool storage_load_coordinate(Coordinate *coordinate);
bool storage_store_coordinate(Coordinate *coordinate);
bool storage_load_time_offset(int8_t *offsets);
bool storage_store_time_offset(int8_t *offsets);
