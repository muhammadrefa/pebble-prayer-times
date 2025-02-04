#include "../models/storage.h"
#include "../models/prayer-times.h"
#include "../models/PrayTimes/prayertimes.h"

#include "../views/window_main.h"
#include "../views/window_schedule.h"
#include "../views/window_settings.h"
#include "../views/window_offset.h"

#include "main_controller.h"

static PTConfig pt_cfg;
static Coordinate coordinate;
struct tm pray_schedule[PRAYERTIMES_TIME_TIMESCOUNT];

int current_offset = 0;

void main_show_offset();

void main_store_settings()
{
    storage_store_pt_config(&pt_cfg);
    storage_store_coordinate(&coordinate);
}

void main_init()
{
    storage_load_pt_config(&pt_cfg);
    storage_load_coordinate(&coordinate);
    pt_init(pt_cfg.calc_method, pt_cfg.juristic, pt_cfg.adjustment);
    pt_set_location(coordinate.latitude, coordinate.longitude);
}

void main_deinit()
{
    pt_deinit();
}

void main_show()
{
    main_window_init(main_show_schedule, main_show_settings);
    main_window_push();
}

void main_show_schedule()
{
    // get prayer times
    time_t tmp = time(NULL);
    struct tm *tick_time = localtime(&tmp);
    pt_set_timezone(tick_time->tm_gmtoff / 3600.0 + (tick_time->tm_isdst > 0 ? 1 : 0));
    pt_get_schedules(tick_time, pray_schedule);

    // set to view
    schedule_window_set_coord(coordinate.latitude, coordinate.longitude);
    schedule_window_set_date(tick_time);
    // for (int i=0; i<PRAYERTIMES_TIME_TIMESCOUNT; ++i)
    //     schedule_window_set_prayer_time(i, pray_schedule+i);
    schedule_window_set_prayer_times(pray_schedule);
    
    schedule_window_set_selected_cb(main_show_offset);

    // show view/window
    schedule_window_push();
}

void main_show_settings()
{
    settings_window_init(main_store_settings, &pt_cfg, &coordinate);
    settings_window_push();
}

void main_show_offset(char* prayer_name, tm* prayer_time)
{
    offset_window_init(prayer_name, prayer_time, &current_offset);
    offset_window_push();
}
