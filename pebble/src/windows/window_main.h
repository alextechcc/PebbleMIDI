#pragma once

typedef void (*ButtonHandler)();

void window_main_push(ButtonHandler pause_handler,
                      ButtonHandler resume_handler,
                      ButtonHandler up_handler,
                      ButtonHandler down_handler,
                      ButtonHandler calibrate_start_handler,
                      ButtonHandler calibrate_end_handler);
void window_main_set_failed();
