#include <pebble.h>
#include "modules/mod_calibrate.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define ACCEL_MIN (-4000)
#define ACCEL_MAX (4000)
#define ACCEL_OUT_MIN (0)
#define ACCEL_OUT_MAX (127)

struct accel_limits {
    int min_x;
    int min_y;
    int min_z;
    int max_x;
    int max_y;
    int max_z;
};

static struct accel_limits s_accel_limits = {
    .min_x = ACCEL_MIN,
    .min_y = ACCEL_MIN,
    .min_z = ACCEL_MIN,
    .max_x = ACCEL_MAX,
    .max_y = ACCEL_MAX,
    .max_z = ACCEL_MAX
};

static uint8_t rescale(int value, int min, int max, int new_min, int new_max) {
    int new_value = (value - min)*(new_max - new_min) / (max - min) + new_min;
    return MIN(MAX(new_value, new_min), new_max);
}

void get_calibrated_value(struct calibrated_accel_data *accel_data) {
    AccelData raw_data;
    accel_service_peek(&raw_data);
    *accel_data = (struct calibrated_accel_data) {
        .x = rescale(raw_data.x, s_accel_limits.min_x, s_accel_limits.max_x,
                ACCEL_OUT_MIN, ACCEL_OUT_MAX),
        .y = rescale(raw_data.y, s_accel_limits.min_y, s_accel_limits.max_y,
                ACCEL_OUT_MIN, ACCEL_OUT_MAX),
        .z = rescale(raw_data.z, s_accel_limits.min_z, s_accel_limits.max_z,
                ACCEL_OUT_MIN, ACCEL_OUT_MAX)
    };
}

void accel_handler(AccelData *data, uint32_t num_samples) {
    s_accel_limits = (struct accel_limits) {
        .min_x = MIN(s_accel_limits.min_x, data[0].x),
        .min_y = MIN(s_accel_limits.min_y, data[0].y),
        .min_z = MIN(s_accel_limits.min_z, data[0].z),
        .max_x = MAX(s_accel_limits.max_x, data[0].x),
        .max_y = MAX(s_accel_limits.max_y, data[0].y),
        .max_z = MAX(s_accel_limits.max_z, data[0].z),
    };
}

static void reset_calibration() {
    s_accel_limits = (struct accel_limits) {
        .min_x = 0,
        .min_y = 0,
        .min_z = 0,
        .max_x = 0,
        .max_y = 0,
        .max_z = 0
    };
}

void calibrate_start() {
    reset_calibration();
    accel_data_service_subscribe(1, accel_handler);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Calibrating started");
}

void calibrate_end() {
    accel_data_service_unsubscribe();
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Calibrating ended");
}

