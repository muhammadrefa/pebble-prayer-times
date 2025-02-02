#include <pebble.h>

#include "controllers/main_controller.h"

void init()
{
    main_init();
    main_show();
}

void deinit()
{
    main_deinit();
}

int main()
{
    init();
    app_event_loop();
    deinit();
}
