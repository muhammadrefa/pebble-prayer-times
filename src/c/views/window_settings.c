#include "../models/storage.h"
#include "../models/PrayTimes/prayertimes.h"
#include "window_settings.h"

static PTConfig *s_pt_cfg;

static Window *s_window;
static StatusBarLayer *s_status_bar;

static SimpleMenuItem s_menu_item[4];
static SimpleMenuSection s_menu_section;
static SimpleMenuLayer *s_menu_layer;

static ActionMenu *s_action_menu;
static ActionMenuLevel *s_action_root_level;
static char str_action_menu_label[10][35];      // max menu count & length

static bool s_changed = false;

static char str_cfg_coord[25] = {0};
static char str_cfg_calc[30] = {0};
static char str_cfg_asr[15] = {0};
static char str_cfg_hi_lat[20] = {0};

static void (*cb_settings_changed)();
static void cb_action_calculation_method(ActionMenu *action_menu, const ActionMenuItem *action, void *context);
static void cb_action_asr(ActionMenu *action_menu, const ActionMenuItem *action, void *context);
static void cb_action_hi_lat(ActionMenu *action_menu, const ActionMenuItem *action, void *context);

static void deinit_action_menu();
static void init_action_menu(int menu_selected);

static void enum2str_calculation_method(PrayerTimes_calculation_method method, char* method_str, int maxlen);
static void enum2str_juristic_method(PrayerTimes_juristic_method method, char* method_str, int maxlen);
static void enum2str_hi_lat_method(PrayerTimes_adjusting_method method, char* method_str, int maxlen);

typedef enum _SettingsMenu
{
    MENU_COORD,
    MENU_CALCULATION_METHOD,
    MENU_ASR,
    MENU_HIGH_LATITUDE
} SettingsMenu;

void settings_window_init(void (*callback_changed)(), PTConfig *pt_cfg, Coordinate *coordinate)
{
    cb_settings_changed = callback_changed;
    s_pt_cfg = pt_cfg;

    enum2str_calculation_method(s_pt_cfg->calc_method, str_cfg_calc, sizeof(str_cfg_calc));
    enum2str_juristic_method(s_pt_cfg->asr, str_cfg_asr, sizeof(str_cfg_asr));
    enum2str_hi_lat_method(s_pt_cfg->hilat_adjustment, str_cfg_hi_lat, sizeof(str_cfg_hi_lat));

    int lat_f = (int)coordinate->latitude;
    unsigned lat_d = abs(coordinate->latitude * 1e6) % 1000000;
    int lng_f = (int)coordinate->longitude;
    unsigned lng_d = abs(coordinate->longitude * 1e6) % 1000000;

    snprintf(str_cfg_coord, sizeof(str_cfg_coord), "%d.%u, %d.%u", lat_f, lat_d, lng_f, lng_d);
}

static void prv_menu_selected(int index, void *context)
{
    if (index == MENU_COORD)
        return;
    deinit_action_menu();
    init_action_menu(index);
    // configure action menu
    ActionMenuConfig am_cfg = (ActionMenuConfig) {
        .root_level = s_action_root_level,
        // .colors = {
        //     .background = PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorWhite),
        //     .foreground = GColorBlack,
        // },
        // .align = ActionMenuAlignCenter
    };

    // show action menu
    s_action_menu = action_menu_open(&am_cfg);
}

static void cb_action_calculation_method(ActionMenu *action_menu, const ActionMenuItem *action, void *context)
{
    // get selected value (from context)
    int value = (int)action_menu_item_get_action_data(action);
    s_pt_cfg->calc_method = (uint8_t)value;
    s_changed = true;

    enum2str_calculation_method(s_pt_cfg->calc_method, str_cfg_calc, sizeof(str_cfg_calc));
}

static void cb_action_asr(ActionMenu *action_menu, const ActionMenuItem *action, void *context)
{
    // get selected value (from context)
    int value = (int)action_menu_item_get_action_data(action);
    s_pt_cfg->asr = (uint8_t)value;
    s_changed = true;

    enum2str_juristic_method(s_pt_cfg->asr, str_cfg_asr, sizeof(str_cfg_asr));
}

static void cb_action_hi_lat(ActionMenu *action_menu, const ActionMenuItem *action, void *context)
{
    // get selected value (from context)
    int value = (int)action_menu_item_get_action_data(action);
    s_pt_cfg->hilat_adjustment = (uint8_t)value;
    s_changed = true;

    enum2str_hi_lat_method(s_pt_cfg->hilat_adjustment, str_cfg_hi_lat, sizeof(str_cfg_hi_lat));
}

static void deinit_action_menu()
{
    if (s_action_menu != NULL)
    {
        action_menu_hierarchy_destroy(s_action_root_level, NULL, NULL);
        s_action_root_level = NULL;
    }
}

static void init_action_menu(int menu_selected)
{
    if (s_action_root_level != NULL)
        deinit_action_menu();

    if (menu_selected == MENU_CALCULATION_METHOD)
    {
        // create action menu entry
        s_action_root_level = action_menu_level_create(6);
        for (int i=0; i<6; ++i)
        {
            enum2str_calculation_method(i, str_action_menu_label[i], sizeof(str_action_menu_label[i]));
            action_menu_level_add_action(s_action_root_level, str_action_menu_label[i], cb_action_calculation_method, (void *)i);
        }
    }

    else if (menu_selected == MENU_ASR)
    {
        // create action menu entry
        s_action_root_level = action_menu_level_create(3);
        for (int i=0; i<2; ++i)
        {
            enum2str_juristic_method(i, str_action_menu_label[i], sizeof(str_action_menu_label[i]));
            action_menu_level_add_action(s_action_root_level, str_action_menu_label[i], cb_action_asr, (void *)i);
        }
    }

    else if (menu_selected == MENU_HIGH_LATITUDE)
    {
        // create action menu entry
        s_action_root_level = action_menu_level_create(4);
        for (int i=0; i<4; ++i)
        {
            enum2str_hi_lat_method(i, str_action_menu_label[i], sizeof(str_action_menu_label[i]));
            action_menu_level_add_action(s_action_root_level, str_action_menu_label[i], cb_action_hi_lat, (void *)i);
        }
    }

    else
        s_action_root_level = action_menu_level_create(0);
}

static void window_load(Window *window)
{
    // get Window info
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // create status bar to show current time
    s_status_bar = status_bar_layer_create();

    // create menu
    int idx = 0;
    s_menu_item[idx++] = (SimpleMenuItem) {.title = "Coordinate", .subtitle = str_cfg_coord, .callback = prv_menu_selected};
    s_menu_item[idx++] = (SimpleMenuItem) {.title = "Method", .subtitle = str_cfg_calc, .callback = prv_menu_selected};
    s_menu_item[idx++] = (SimpleMenuItem) {.title = "Asr", .subtitle = str_cfg_asr, .callback = prv_menu_selected};
    s_menu_item[idx++] = (SimpleMenuItem) {.title = "Hi latitude adj.", .subtitle = str_cfg_hi_lat, .callback = prv_menu_selected};
    s_menu_section.title = NULL;
    s_menu_section.items = s_menu_item;
    s_menu_section.num_items = 4;
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
    if (s_changed)
        cb_settings_changed();
    if (s_action_menu != NULL)
        deinit_action_menu();
    simple_menu_layer_destroy(s_menu_layer);
    status_bar_layer_destroy(s_status_bar);
    window_destroy(s_window);
    s_window = NULL;
}

void settings_window_push()
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

// max 35
static void enum2str_calculation_method(PrayerTimes_calculation_method method, char* method_str, int maxlen)
{
    switch (method)
    {
        case PRAYERTIMES_CALCULATION_JAFARI:
            snprintf(method_str, maxlen, "Ja'fari"); break;
        case PRAYERTIMES_CALCULATION_KARACHI:
            snprintf(method_str, maxlen, "Karachi"); break;
        case PRAYERTIMES_CALCULATION_ISNA:
            snprintf(method_str, maxlen, "Islamic Society of North America"); break;
        case PRAYERTIMES_CALCULATION_MWL:
            snprintf(method_str, maxlen, "Muslim World League"); break;
        case PRAYERTIMES_CALCULATION_MAKKAH:
            snprintf(method_str, maxlen, "Makkah"); break;
        case PRAYERTIMES_CALCULATION_EGYPT:
            snprintf(method_str, maxlen, "Egypt"); break;
        case PRAYERTIMES_CALCULATION_CUSTOM:
            snprintf(method_str, maxlen, "Custom"); break;
        default:
            snprintf(method_str, maxlen, "Unknown (%d)", method); break;
    }
}

// max 15
static void enum2str_juristic_method(PrayerTimes_juristic_method method, char* method_str, int maxlen)
{
    switch (method)
    {
        case PRAYERTIMES_JURISTIC_SHAFII:
            snprintf(method_str, maxlen, "Standard"); break;
        case PRAYERTIMES_JURISTIC_HANAFI:
            snprintf(method_str, maxlen, "Hanafi"); break;
        default:
            snprintf(method_str, maxlen, "Unknown (%d)", method); break;
    }
}

// max 20
static void enum2str_hi_lat_method(PrayerTimes_adjusting_method method, char* method_str, int maxlen)
{
    switch (method)
    {
        case PRAYERTIMES_ADJUSTMENT_NONE:
            snprintf(method_str, maxlen, "No adjustment"); break;
        case PRAYERTIMES_ADJUSTMENT_MIDNIGHT:
            snprintf(method_str, maxlen, "Midnight"); break;
        case PRAYERTIMES_ADJUSTMENT_ONESEVENTH:
            snprintf(method_str, maxlen, "1/7th of the night"); break;
        case PRAYERTIMES_ADJUSTMENT_ANGLEBASED:
            snprintf(method_str, maxlen, "Angle-based"); break;
        default:
            snprintf(method_str, maxlen, "Unknown (%d)", method); break;
    }
}
