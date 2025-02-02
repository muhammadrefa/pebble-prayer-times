#include "window_main.h"

static Window *s_window;
static StatusBarLayer *s_status_bar;

static SimpleMenuItem s_menu_item[2];
static SimpleMenuSection s_menu_section;
static SimpleMenuLayer *s_menu_layer;

static void (*cb_menu_schedule)();
static void (*cb_menu_settings)();

static void prv_menu_selected(int index, void *context)
{
    switch (index)
    {
        case 0: if (cb_menu_schedule != NULL) cb_menu_schedule(); break;
        case 1: if (cb_menu_settings != NULL) cb_menu_settings(); break;
    }
}

static void window_load(Window *window)
{
    // get Window info
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // create status bar to show current time
    s_status_bar = status_bar_layer_create();

    // create menu
    s_menu_item[0].title = "Schedule";
    s_menu_item[0].callback = prv_menu_selected;
    s_menu_item[1].title = "Settings";
    s_menu_item[1].callback = prv_menu_selected;
    s_menu_section.title = NULL;
    s_menu_section.items = s_menu_item;
    s_menu_section.num_items = 2;
    s_menu_layer = simple_menu_layer_create(
        GRect(0, STATUS_BAR_LAYER_HEIGHT, bounds.size.w, bounds.size.h-STATUS_BAR_LAYER_HEIGHT),
        window,
        &s_menu_section,
        1,
        NULL
    );

    // add as child layer
    layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
    layer_add_child(window_layer, simple_menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window)
{
    simple_menu_layer_destroy(s_menu_layer);
    status_bar_layer_destroy(s_status_bar);
    window_destroy(s_window);
    s_window = NULL;
}

void main_window_init(void (*callback_schedule)(), void(*callback_settings)())
{
    cb_menu_schedule = callback_schedule;
    cb_menu_settings = callback_settings;
}

void main_window_push()
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
