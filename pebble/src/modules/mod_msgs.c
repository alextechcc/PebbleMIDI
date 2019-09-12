#include <pebble.h>
#include "modules/mod_msgs.h"
#include "modules/mod_comm.h"
#include "modules/mod_calibrate.h"

static bool s_up_pressed = false;
static bool s_down_pressed = false;

void send_callback() {
    DictionaryIterator *out = comm_begin();

    struct calibrated_accel_data accel_data;
    get_calibrated_value(&accel_data);

    // Don't check for errors as we know we have enough space, and it's valid
    // (we repeat send only when the last message failed)
    dict_write_uint8(out, 0, s_up_pressed << 1 | s_down_pressed << 0);
    dict_write_uint8(out, 1, accel_data.x);
    dict_write_uint8(out, 2, accel_data.y);
    dict_write_uint8(out, 3, accel_data.z);
    comm_send();

    s_up_pressed = false;
    s_down_pressed = false;
}

void send_up() {
    s_up_pressed = true;
}

void send_down() {
    s_down_pressed = true;
}

