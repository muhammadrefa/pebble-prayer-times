#include "window_offset.h"
#include "layers/selection_layer.h"

static Window *s_window;
static StatusBarLayer *s_status_bar;
static Layer *s_selection_layer;

static const unsigned total_input = 3;
static const int input_min_max[2] = {-60, 60};

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
}

static void window_load(Window *window)
{
    // get Window info
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // create status bar to show current time
    s_status_bar = status_bar_layer_create();

    // creaete selection layer
    const GEdgeInsets selection_insets = GEdgeInsets(
        (bounds.size.h - PIN_WINDOW_SIZE.h) / 2,
        (bounds.size.w - PIN_WINDOW_SIZE.w) / 2
    );

    s_selection_layer = selection_layer_create(grect_inset(bounds, selection_insets), total_input);
    selection_layer_set_cell_width(s_selection_layer, 0, 40);
    selection_layer_set_cell_width(s_selection_layer, 1, 40);
    selection_layer_set_cell_width(s_selection_layer, 2, 40);
    
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
    layer_add_child(window_layer, s_selection_layer);
}

static void window_unload(Window *window)
{
    layer_destroy(s_selection_layer);
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
