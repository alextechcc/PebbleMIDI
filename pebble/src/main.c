#include <pebble.h>
#include <modules/mod_comm.h>
#include <windows/window_main.h>
#include <modules/mod_msgs.h>
#include <modules/mod_calibrate.h>

void failed_handler() {
    window_main_set_failed();
    comm_pause();
}

static void init() {
    window_main_push(comm_pause, comm_resume, send_up, send_down,
            calibrate_start, calibrate_end);
    comm_open(send_callback, failed_handler);
}

static void deinit() {
    comm_close();
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
