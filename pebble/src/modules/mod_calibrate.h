#pragma once

struct calibrated_accel_data {
    uint8_t x;
    uint8_t y;
    uint8_t z;
};

void calibrate_start();

void calibrate_end();

void get_calibrated_value(struct calibrated_accel_data *accel_data);
