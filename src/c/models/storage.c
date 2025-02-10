#include <pebble.h>

#include "PrayTimes/prayertimes.h"
#include "storage.h"

bool storage_load_pt_config(PTConfig *settings)
{
    status_t res = persist_read_data(STORAGE_PT_CONFIG, (void *)settings, sizeof(PTConfig));
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "load=%d", (int)res);

    // TODO: Set default settings
    if (res != sizeof(PTConfig))
    {
        // match PrayTimes example
        *settings = (PTConfig){
            .calc_method = PRAYERTIMES_CALCULATION_MWL,
            .asr = PRAYERTIMES_JURISTIC_SHAFII,
            .hilat_adjustment = PRAYERTIMES_ADJUSTMENT_NONE,
            .use_offset = false
        };

        // if (res == E_DOES_NOT_EXIST)
        //     storage_store_pt_config(settings);
    }
    else
    {
        // APP_LOG(APP_LOG_LEVEL_DEBUG, "load");
        // APP_LOG(APP_LOG_LEVEL_DEBUG, "method=%d", (int)settings->calc_method);
        // APP_LOG(APP_LOG_LEVEL_DEBUG, "adjust=%d", (int)settings->adjustment);
    }
    // return res != E_DOES_NOT_EXIST;
    return true;
}

bool storage_store_pt_config(PTConfig *settings)
{
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "store");
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "method=%d", (int)settings->calc_method);
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "adjust=%d", (int)settings->adjustment);
    int res = persist_write_data(STORAGE_PT_CONFIG, (void *)settings, sizeof(PTConfig));
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "stor=%d", (int)res);
    return res == sizeof(settings);
}

bool storage_load_coordinate(Coordinate *coordinate)
{
    status_t res = persist_read_data(STORAGE_COORDINATE, (void *)coordinate, sizeof(Coordinate));

    // TODO: Set default settings
    if (res != sizeof(Coordinate))
    {
        coordinate->latitude = 0.0;
        coordinate->longitude = 0.0;
    }
    else
    {

    }
    return true;
}

bool storage_store_coordinate(Coordinate *coordinate)
{
    int res = persist_write_data(STORAGE_COORDINATE, (void *)coordinate, sizeof(Coordinate));
    return res == sizeof(Coordinate);
}

bool storage_load_time_offset(int8_t *offsets)
{
    status_t res = persist_read_data(STORAGE_TIME_OFFSET, (void *)offsets, sizeof(int8_t)*7);
    // return res != E_DOES_NOT_EXIST;
    return true;
}

bool storage_store_time_offset(int8_t *offsets)
{
    status_t res = persist_write_data(STORAGE_TIME_OFFSET, offsets, sizeof(int8_t)*7);
    // return res == S_SUCCESS;
    return res == sizeof(int8_t)*7;
}
