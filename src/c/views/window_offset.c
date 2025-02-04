#include "view_utils.h"
#include "layers/selection_layer.h"
#include "window_offset.h"

static Window *s_window;
static StatusBarLayer *s_status_bar;
static Layer *s_selection_layer;

const unsigned total_input = 3;
const int input_min_max[2] = {-60, 60};
const unsigned string_size = 10 + 1;    // NULL terminated

typedef struct _InputData
{
    int value;
    char text[3][2];
} InputData;

typedef struct _ShowValueLayer
{
    Layer* layer;
    TextLayer* title;
    TextLayer* subtitle;
    TextLayer* value;
} ShowValueLayer;

static InputData *s_input_data;
static int8_t *current_offset;

static int prayer_idx;
static tm* prayer_time;

static ShowValueLayer* s_original_time;
static char *s_original_time_str;

static ShowValueLayer* s_offset_time;
static char *s_offset_time_str;

static void (*cb_store)(int, int);

// LSD in position 0
static unsigned prv_get_num(unsigned position, int number)
{
    unsigned _pos = 1;
    for (unsigned i=0; i<position; ++i)   // power
        _pos *= 10;
    return (abs(number) / _pos) % 10;
}

/*
static void prv_set_num(unsigned position, unsigned number_to_set, int *target)
{
    APP_LOG(APP_LOG_LEVEL_DEBUG, "set p=%d v=%d t=%d", position, number_to_set, *target);
    number_to_set %= 10;
    bool is_neg = *target < 0;
    int delta = number_to_set - prv_get_num(position, *target);
    *target = abs(*target) + (delta * 10^(position-1));
    *target *= (is_neg ? -1 : 1);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "res p=%d v=%d t=%d", position, number_to_set, *target);
}
*/

static ShowValueLayer* prv_show_value_layer_create(GRect frame)
{
    ShowValueLayer* new_layer = malloc(sizeof(ShowValueLayer));
    new_layer->layer = layer_create(frame);

    unsigned divider_width = frame.size.w / 5;
    unsigned divider_height = frame.size.h / 3;

    new_layer->title = text_layer_create(GRect(0, 0, divider_width*2, frame.size.h-divider_height));
    // text_layer_set_background_color(new_layer->title, GColorBabyBlueEyes);
    text_layer_set_text_color(new_layer->title, GColorBlack);
    text_layer_set_text(new_layer->title, "Title");
    text_layer_set_font(new_layer->title, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text_alignment(new_layer->title, GTextAlignmentLeft);

    // new_layer->subtitle = text_layer_create(GRect(0, frame.size.h-divider_height, divider_width*2, divider_height));
    new_layer->subtitle = text_layer_create(GRect(0, text_layer_get_content_size(new_layer->title).h + 5, divider_width*2, divider_height));
    // text_layer_set_background_color(new_layer->subtitle, GColorRoseVale);
    text_layer_set_text_color(new_layer->subtitle, GColorBlack);
    text_layer_set_text(new_layer->subtitle, "Subtitle");
    text_layer_set_font(new_layer->subtitle, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_text_alignment(new_layer->subtitle, GTextAlignmentLeft);

    new_layer->value = text_layer_create(GRect(divider_width*2, 0, frame.size.w-(divider_width*2), frame.size.h));
    // snprintf(new_layer->time, string_size, "%d%d:%d%d am", myval, myval+1, myval+2, myval+3);
    // text_layer_set_background_color(new_layer->value, GColorCyan);
    text_layer_set_text_color(new_layer->value, GColorBlack);
    text_layer_set_text(new_layer->value, "Value");
    text_layer_set_font(new_layer->value, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment(new_layer->value, GTextAlignmentRight);

    layer_add_child(new_layer->layer, text_layer_get_layer(new_layer->title));
    layer_add_child(new_layer->layer, text_layer_get_layer(new_layer->subtitle));
    layer_add_child(new_layer->layer, text_layer_get_layer(new_layer->value));

    return new_layer;
}

static void prv_show_value_layer_destroy(ShowValueLayer* layer)
{
    text_layer_destroy(layer->title);
    text_layer_destroy(layer->subtitle);
    text_layer_destroy(layer->value);
    layer_destroy(layer->layer);
    free(layer);
}

static Layer* prv_show_value_layer_get_layer(ShowValueLayer* layer)
{
    return layer->layer;
}

static void prv_show_value_layer_set_title(ShowValueLayer* layer, const char* title)
{
    text_layer_set_text(layer->title, title);
}

static void prv_show_value_layer_set_subtitle(ShowValueLayer* layer, const char* subtitle)
{
    text_layer_set_text(layer->subtitle, subtitle);
}

static void prv_show_value_layer_set_value(ShowValueLayer* layer, char* value)
{
    text_layer_set_text(layer->value, value);
}

static void prv_update_offset_time_preview(int offset)
{
    tm offset_time = view_util_apply_offset(prayer_time, offset);
    strftime(s_offset_time_str, string_size, clock_is_24h_style() ? "%H:%M" : "%I:%M %p", &offset_time);
}

static char* selection_handle_get_text(int index, void *context) {
    InputData *input_data = (InputData*)context;

    if (index == 0)
    {
        snprintf(input_data->text[index], 2, "%s", input_data->value < 0 ? "-" : "+");
    }
    else
    {
        unsigned number_position = total_input - 1 - index;
        int num = prv_get_num(number_position, input_data->value);
        snprintf(input_data->text[index], 2, "%d", num);
    }
    return input_data->text[index];
}

static void selection_handle_complete(void *context) {
    cb_store(prayer_idx, s_input_data->value);

    window_stack_pop(true);
}

static void selection_handle_inc(int index, uint8_t clicks, void *context) {
    InputData *input_data = (InputData*)context;

    if (index == 0)
    {
        input_data->value *= (-1);
    }
    else
    {
        unsigned number_position = total_input - 1 - index;

        unsigned to_add = 1;
        for (unsigned i=0; i<number_position; ++i)   // power
            to_add *= 10;
        input_data->value += to_add;

        if (input_data->value > input_min_max[1])
            input_data->value -= to_add;        // undo operation
    }
    prv_update_offset_time_preview(input_data->value);
}

static void selection_handle_dec(int index, uint8_t clicks, void *context) {
    InputData *input_data = (InputData*)context;

    if (index == 0)
    {
        input_data->value *= (-1);
    }
    else
    {
        unsigned number_position = total_input - 1 - index;

        unsigned to_sub = 1;
        for (unsigned i=0; i<number_position; ++i)   // power
            to_sub *= 10;
        input_data->value -= to_sub;

        if (input_data->value < input_min_max[0])
            input_data->value += to_sub;        // undo operation
    }
    prv_update_offset_time_preview(input_data->value);
}

static void window_load(Window *window)
{
    s_original_time_str = malloc(sizeof(char)*string_size);
    s_offset_time_str = malloc(sizeof(char)*string_size);

    if (s_input_data == NULL)
        s_input_data = malloc(sizeof(InputData));
    s_input_data->value = *current_offset;

    // get Window info
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // create status bar to show current time
    s_status_bar = status_bar_layer_create();

    const unsigned available_height = bounds.size.h - STATUS_BAR_LAYER_HEIGHT;
    const unsigned widget_height = available_height / 3;

    // create layer to show the original time
    s_original_time = prv_show_value_layer_create(GRect(0, 0+STATUS_BAR_LAYER_HEIGHT, bounds.size.w, widget_height));
    prv_show_value_layer_set_title(s_original_time, view_util_time_name[prayer_idx]);
    prv_show_value_layer_set_subtitle(s_original_time, "(original)");
    prv_show_value_layer_set_value(s_original_time, s_original_time_str);
    // snprintf(s_original_time_str, string_size, "%d%d:%d%d am", myval, myval+1, myval+2, myval+3);
    strftime(s_original_time_str, string_size, clock_is_24h_style() ? "%H:%M" : "%I:%M %p", prayer_time);

    // create layer to show the offset-ed time
    s_offset_time = prv_show_value_layer_create(GRect(0, 2*widget_height+STATUS_BAR_LAYER_HEIGHT, bounds.size.w, widget_height));
    prv_show_value_layer_set_title(s_offset_time, view_util_time_name[prayer_idx]);
    prv_show_value_layer_set_subtitle(s_offset_time, "(offset)");
    prv_show_value_layer_set_value(s_offset_time, s_offset_time_str);
    // snprintf(s_offset_time_str, string_size, "%d", myval+s_input_data->value);
    // strftime(s_offset_time_str, string_size, clock_is_24h_style() ? "%H:%M" : "%I:%M %p", prayer_time_offset);
    prv_update_offset_time_preview(*current_offset);

    // create selection layer
    GRect input_bounds = GRect(0, 0+STATUS_BAR_LAYER_HEIGHT+widget_height, bounds.size.w, available_height/3);
    const GEdgeInsets selection_insets = GEdgeInsets(
        (input_bounds.size.h - PIN_WINDOW_SIZE.h) / 2,
        (input_bounds.size.w - PIN_WINDOW_SIZE.w) / 2
    );

    s_selection_layer = selection_layer_create(grect_inset(input_bounds, selection_insets), total_input);
    for (unsigned i=0; i<total_input; ++i)
        selection_layer_set_cell_width(s_selection_layer, i, 40);
    
    selection_layer_set_cell_padding(s_selection_layer, 4);
    selection_layer_set_active_bg_color(s_selection_layer, GColorRed);
    selection_layer_set_inactive_bg_color(s_selection_layer, GColorDarkGray);
    selection_layer_set_click_config_onto_window(s_selection_layer, window);
    selection_layer_set_callbacks(s_selection_layer, s_input_data, (SelectionLayerCallbacks) {
        .get_cell_text = selection_handle_get_text,
        .complete = selection_handle_complete,
        .increment = selection_handle_inc,
        .decrement = selection_handle_dec,
    });

    // add as child layer
    layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
    layer_add_child(window_layer, prv_show_value_layer_get_layer(s_original_time));
    layer_add_child(window_layer, prv_show_value_layer_get_layer(s_offset_time));
    layer_add_child(window_layer, s_selection_layer);
}

static void window_unload(Window *window)
{
    layer_destroy(s_selection_layer);
    prv_show_value_layer_destroy(s_original_time);
    prv_show_value_layer_destroy(s_offset_time);
    status_bar_layer_destroy(s_status_bar);
    window_destroy(s_window);
    s_window = NULL;

    free(s_original_time_str);
    free(s_offset_time_str);
    
    if (s_input_data != NULL)
    {
        free(s_input_data);
        s_input_data = NULL;
    }
}

void offset_window_init(int _prayer_idx, tm* _prayer_time, int8_t* _current_offset, void (*callback_store)(int, int))
{
    prayer_idx = _prayer_idx;
    prayer_time = _prayer_time;
    current_offset = _current_offset;
    cb_store = callback_store;
}

void offset_window_push()
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
