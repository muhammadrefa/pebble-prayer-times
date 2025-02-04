#include <stdlib.h>
#include "window_schedule.h"

static Window *s_window;
static StatusBarLayer *s_status_bar;

static SimpleMenuItem s_menu_item[7];
static SimpleMenuSection s_menu_section;
static SimpleMenuLayer *s_menu_layer;

static char coord_str[25] = {0};
static TextLayer *s_coord_lyr;

static char date_str[30] = {0};
static TextLayer *s_date_lyr;

static tm* prayer_time;
static char time_str[7][10];
static const char *time_name[] = {"Fajr", "Sunrise", "Dhuhr", "Asr", "Sunset", "Maghrib", "Isha"};

static void (*cb_schedule_selected)(char*, tm*);

static void prv_menu_selected(int index, void *context)
{
    if (cb_schedule_selected)
        cb_schedule_selected((char *)time_name[index], prayer_time+index);
}

static void window_load(Window *window)
{
    // get Window info
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // create status bar to show current time
    s_status_bar = status_bar_layer_create();

    // header
    s_coord_lyr = text_layer_create(GRect(0, 0+STATUS_BAR_LAYER_HEIGHT, bounds.size.w, 15));
    text_layer_set_background_color(s_coord_lyr, GColorClear);
    text_layer_set_text_color(s_coord_lyr, GColorBlack);
    text_layer_set_text(s_coord_lyr, coord_str);
    text_layer_set_font(s_coord_lyr, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    s_date_lyr = text_layer_create(GRect(0, 0+STATUS_BAR_LAYER_HEIGHT+15, bounds.size.w, 15));
    text_layer_set_background_color(s_date_lyr, GColorClear);
    text_layer_set_text_color(s_date_lyr, GColorBlack);
    text_layer_set_text(s_date_lyr, date_str);
    text_layer_set_font(s_date_lyr, fonts_get_system_font(FONT_KEY_GOTHIC_14));

    // create menu
    for (int i=0; i<7; i++)
    {
        s_menu_item[i].title = time_str[i];
        s_menu_item[i].subtitle = time_name[i];
        s_menu_item[i].callback = prv_menu_selected;
    }
    s_menu_section.title = NULL;
    s_menu_section.items = s_menu_item;
    s_menu_section.num_items = 7;
    s_menu_layer = simple_menu_layer_create(
        GRect(0, STATUS_BAR_LAYER_HEIGHT+30, bounds.size.w, bounds.size.h-STATUS_BAR_LAYER_HEIGHT-30),
        window,
        &s_menu_section,
        1,
        NULL
    );

    // add as child layer
    layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
    layer_add_child(window_layer, text_layer_get_layer(s_coord_lyr));
    layer_add_child(window_layer, text_layer_get_layer(s_date_lyr));
    layer_add_child(window_layer, simple_menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window)
{
    simple_menu_layer_destroy(s_menu_layer);
    text_layer_destroy(s_date_lyr);
    text_layer_destroy(s_coord_lyr);
    status_bar_layer_destroy(s_status_bar);
    window_destroy(s_window);
    s_window = NULL;
}

// int schedule_window_set_prayer_time(int idx, tm *_time)
// {
//     return strftime(time_str[idx], sizeof(time_str[idx]), clock_is_24h_style() ? "%H:%M" : "%I:%M %p", _time);
// }

void schedule_window_set_prayer_times(tm *_time)
{
    prayer_time = _time;
    for (unsigned i=0; i<7; ++i)
        strftime(time_str[i], sizeof(time_str[i]), clock_is_24h_style() ? "%H:%M" : "%I:%M %p", _time+i);
}

int schedule_window_set_date(tm *_time)
{
    return strftime(date_str, sizeof(date_str), "Schedule for %d %b %Y", _time);
}

void schedule_window_set_coord(double lat, double lng)
{
    int lat_f = (int)lat;
    unsigned lat_d = abs(lat * 1e6) % 1000000;
    int lng_f = (int)lng;
    unsigned lng_d = abs(lng * 1e6) % 1000000;

    snprintf(coord_str, sizeof(coord_str), "%d.%u, %d.%u", lat_f, lat_d, lng_f, lng_d);
}

void schedule_window_set_selected_cb(void (*callback_selected)())
{
    cb_schedule_selected = callback_selected;
}

void schedule_window_push()
{
    if (!s_window)
    {
        s_window = window_create();
        window_set_window_handlers(s_window, (WindowHandlers) {
            .load = window_load,
            .unload = window_unload
        });
    }

    window_stack_push(s_window, true);
}
