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
static int8_t time_offset[PRAYERTIMES_TIME_TIMESCOUNT];

static bool is_settings_changed = false;
static bool is_offset_changed = false;

void main_show_offset();

void prv_main_appconfig_received_handler(DictionaryIterator *iter, void *context)
{
    APP_LOG(APP_LOG_LEVEL_DEBUG, "AppMessage received");

    Tuple *received_cfg;
    
    received_cfg = dict_find(iter, MESSAGE_KEY_CoordinateLatitude);
    if (received_cfg)
        coordinate.latitude = atoi(received_cfg->value->cstring) / 1000000.0;
        // APP_LOG(APP_LOG_LEVEL_DEBUG, "CoordinateLatitude %d %d", received_cfg->type, atoi(received_cfg->value->cstring));

    received_cfg = dict_find(iter, MESSAGE_KEY_CoordinateLongitude);
    if (received_cfg)
        coordinate.longitude = atoi(received_cfg->value->cstring) / 1000000.0;
        // APP_LOG(APP_LOG_LEVEL_DEBUG, "CoordinateLongitude %d %d", received_cfg->type, atoi(received_cfg->value->cstring));

    received_cfg = dict_find(iter, MESSAGE_KEY_SettingsMethod);
    if (received_cfg)
        pt_cfg.calc_method = atoi(received_cfg->value->cstring);
        // APP_LOG(APP_LOG_LEVEL_DEBUG, "SettingsMethod %d %d", received_cfg->type, atoi(received_cfg->value->cstring));

    received_cfg = dict_find(iter, MESSAGE_KEY_SettingsAsr);
    if (received_cfg)
        pt_cfg.asr = atoi(received_cfg->value->cstring);
        // APP_LOG(APP_LOG_LEVEL_DEBUG, "SettingsAsr %d %d", received_cfg->type, atoi(received_cfg->value->cstring));

    received_cfg = dict_find(iter, MESSAGE_KEY_SettingsHiLatitude);
    if (received_cfg)
        pt_cfg.hilat_adjustment = atoi(received_cfg->value->cstring);
        // APP_LOG(APP_LOG_LEVEL_DEBUG, "SettingsHiLatitude %d %d", received_cfg->type, atoi(received_cfg->value->cstring));

    is_settings_changed = true;
    
    main_show_settings();
}

void prv_main_appconfig_dropped_handler(AppMessageResult reason, void *context)
{
    APP_LOG(APP_LOG_LEVEL_DEBUG, "AppMessage dropped! Reason:%d", reason);
}

void main_settings_changed()
{
    is_settings_changed = true;
}

void main_offset_changed(int prayer_idx, int offset)
{
    time_offset[prayer_idx] = (int8_t)offset;
    is_offset_changed = true;
}

void main_init()
{
    storage_load_pt_config(&pt_cfg);
    storage_load_coordinate(&coordinate);
    storage_load_time_offset(time_offset);
    pt_init(pt_cfg.calc_method, pt_cfg.asr, pt_cfg.hilat_adjustment);
    pt_set_location(coordinate.latitude, coordinate.longitude);

    // Open AppMessage connection
    app_message_register_inbox_received(prv_main_appconfig_received_handler);
    app_message_register_inbox_dropped(prv_main_appconfig_dropped_handler);
    app_message_open(128, 128);
}

void main_deinit()
{
    pt_deinit();

    if (is_settings_changed)
    {
        storage_store_pt_config(&pt_cfg);
        storage_store_coordinate(&coordinate);
    }

    if (is_offset_changed)
    {
        storage_store_time_offset(time_offset);
    }
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
    schedule_window_init();
    schedule_window_set_coord(coordinate.latitude, coordinate.longitude);
    schedule_window_set_date(tick_time);
    schedule_window_set_prayer_times(pray_schedule, time_offset);
    
    schedule_window_set_selected_cb(main_show_offset);

    // show view/window
    schedule_window_push();
}

void main_show_settings()
{
    settings_window_init(main_settings_changed, &pt_cfg, &coordinate);
    settings_window_push();
}

void main_show_offset(int prayer_idx)
{
    offset_window_init(prayer_idx, &pray_schedule[prayer_idx], (int8_t *)&time_offset[prayer_idx], main_offset_changed);
    offset_window_push();
}
