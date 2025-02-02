#include <time.h>
#include <pebble.h>
#include "PrayTimes/prayertimes.h"

#include "prayer-times.h"

static PrayerTimes* pt;
static double lat, lng = 0;
static double tz = 0;

void pt_init(PrayerTimes_calculation_method method, PrayerTimes_juristic_method asr_method, PrayerTimes_adjusting_method hi_lat_adjusment)
{
    pt = PrayerTimes_create();
    PrayerTimes_set_calc_method(pt, method);
    PrayerTimes_set_asr_method(pt, asr_method);
    PrayerTimes_set_high_lats_adjust_method(pt, hi_lat_adjusment);
}

void pt_deinit()
{
    PrayerTimes_destroy(pt);
}

void pt_set_location(double latitude, double longitude)
{
    lat = latitude;
    lng = longitude;
}

void pt_set_timezone(double timezone)
{
    tz = timezone;
}

void pt_get_schedules0(tm *time, double* times)
{
    APP_LOG(APP_LOG_LEVEL_DEBUG, "year %u month %u day %u", 1900+time->tm_year, 1+time->tm_mon, time->tm_mday);
    PrayerTimes_get_prayer_times(pt, 1900+time->tm_year, 1+time->tm_mon, time->tm_mday, lat, lng, tz, times);
}

void pt_get_schedules(tm *time, tm* schedule)
{
    double times[PRAYERTIMES_TIME_TIMESCOUNT];
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "year %u month %u day %u", 1900+time->tm_year, 1+time->tm_mon, time->tm_mday);
    PrayerTimes_get_prayer_times(pt, 1900+time->tm_year, 1+time->tm_mon, time->tm_mday, lat, lng, tz, times);

    for (int i=0; i<PRAYERTIMES_TIME_TIMESCOUNT; ++i)
    {
        int h, m = 0;
        PrayerTimes_get_float_time_parts(times[i], &h, &m);
        *(schedule+i) = *time;
        (schedule+i)->tm_hour = h;
        (schedule+i)->tm_min = m;
        (schedule+i)->tm_sec = 0;

        // APP_LOG(APP_LOG_LEVEL_DEBUG, "h=%d m=%d %d %d %d", h, m, (schedule+i)->tm_hour, (schedule+i)->tm_min, (schedule+i)->tm_sec);
    }
}

void pt_time2str(char *time_str, bool is_24h, double time)
{
    if (is_24h)
        PrayerTimes_float_time_to_time24(time_str, time);
    else
        PrayerTimes_float_time_to_time12(time_str, time, 0);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "time %s", time_str);
}
