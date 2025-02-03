#include "window_offset.h"
#include "layers/selection_layer.h"

static Window *s_window;
static StatusBarLayer *s_status_bar;
static Layer *s_selection_layer;

static TextLayer *s_original_time_layer;
static TextLayer *s_offset_time_layer;
static char s_original_time_str[10];
static char s_offset_time_str[10];

static const unsigned total_input = 3;
static const int input_min_max[2] = {-60, 60};

static int myval = 4;

typedef struct _InputData
{
    int value;
    char text[3][2];
} InputData;

// TODO: Input data shouldn't be here
static InputData s_input_data = {.value = 0};

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
    APP_LOG(APP_LOG_LEVEL_DEBUG, "selection completed");
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
    snprintf(s_offset_time_str, sizeof(s_offset_time_str), "%d", myval + s_input_data.value);
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
    snprintf(s_offset_time_str, sizeof(s_offset_time_str), "%d", myval + s_input_data.value);
}

static void window_load(Window *window)
{
    // get Window info
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // create status bar to show current time
    s_status_bar = status_bar_layer_create();

    const unsigned available_height = bounds.size.h - STATUS_BAR_LAYER_HEIGHT;
    const unsigned widget_height = available_height / 3;

    // create layer to show the original time
    s_original_time_layer = text_layer_create(GRect(0, 0+STATUS_BAR_LAYER_HEIGHT, bounds.size.w, available_height/3));
    snprintf(s_original_time_str, sizeof(s_original_time_str), "%d", myval);
    text_layer_set_background_color(s_original_time_layer, GColorCyan);
    text_layer_set_text_color(s_original_time_layer, GColorBlack);
    text_layer_set_text(s_original_time_layer, s_original_time_str);
    text_layer_set_font(s_original_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    text_layer_set_text_alignment(s_original_time_layer, GTextAlignmentCenter);

    // create layer to show the offset-ed time
    s_offset_time_layer = text_layer_create(GRect(0, 2*widget_height+STATUS_BAR_LAYER_HEIGHT, bounds.size.w, available_height/3));
    snprintf(s_offset_time_str, sizeof(s_offset_time_str), "%d", myval + s_input_data.value);
    text_layer_set_background_color(s_offset_time_layer, GColorArmyGreen);
    text_layer_set_text_color(s_offset_time_layer, GColorBlack);
    text_layer_set_text(s_offset_time_layer, s_offset_time_str);
    text_layer_set_font(s_offset_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    text_layer_set_text_alignment(s_offset_time_layer, GTextAlignmentCenter);

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
    selection_layer_set_callbacks(s_selection_layer, &s_input_data, (SelectionLayerCallbacks) {
        .get_cell_text = selection_handle_get_text,
        .complete = selection_handle_complete,
        .increment = selection_handle_inc,
        .decrement = selection_handle_dec,
    });

    // add as child layer
    layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
    layer_add_child(window_layer, text_layer_get_layer(s_original_time_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_offset_time_layer));
    layer_add_child(window_layer, s_selection_layer);
}

static void window_unload(Window *window)
{
    layer_destroy(s_selection_layer);
    text_layer_destroy(s_original_time_layer);
    text_layer_destroy(s_offset_time_layer);
    status_bar_layer_destroy(s_status_bar);
    window_destroy(s_window);
    s_window = NULL;
}

void offset_window_init()
{

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
