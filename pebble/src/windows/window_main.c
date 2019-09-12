#include "pebble.h"
#include "windows/window_main.h"

static Window *s_main_window;
static TextLayer *s_main_text_layer, *s_sub_text_layer;
static StatusBarLayer *s_status_bar_layer;
static ActionBarLayer *s_action_bar_layer;
static GBitmap *s_play_bitmap, *s_pause_bitmap, *s_up_bitmap, *s_down_bitmap,
               *s_up_disabled_bitmap, *s_down_disabled_bitmap, *s_stop_bitmap;
static ButtonHandler s_pause_handler, s_resume_handler, s_up_handler, s_down_handler,
                     s_calibrate_start_handler, s_calibrate_end_handler;

enum state {
    resumed,
    paused,
    calibrating,
    failed
};

static int s_state;

static const char *resume_str = "PEBBLE\nMIDI\nSending";
static const char *paused_str = "PEBBLE\nMIDI\nPaused";
static const char *calibrating_str = "PEBBLE\nMIDI\nCalibrating";
static const char *failed_str = "PEBBLE\nMIDI\nFailed";

static void set_status_display() {
    switch (s_state) {
        case resumed:
            action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_UP, s_up_bitmap);
            action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_DOWN, s_down_bitmap);
            action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_SELECT, s_pause_bitmap);
            text_layer_set_text(s_main_text_layer, resume_str);
            text_layer_set_text(s_sub_text_layer, "");
            window_set_background_color(s_main_window, GColorJaegerGreen);
            break;

        case paused:
            action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_UP, s_up_disabled_bitmap);
            action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_DOWN, s_down_disabled_bitmap);
            action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_SELECT, s_play_bitmap);
            text_layer_set_text(s_main_text_layer, paused_str);
            text_layer_set_text(s_sub_text_layer, "Double Press Select to Calibrate");
            window_set_background_color(s_main_window, GColorCeleste);
            break;

        case calibrating:
            action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_UP, s_up_disabled_bitmap);
            action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_DOWN, s_down_disabled_bitmap);
            action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_SELECT, s_stop_bitmap);
            text_layer_set_text(s_main_text_layer, calibrating_str);
            text_layer_set_text(s_sub_text_layer, "Press Select to Stop Calibrating");
            window_set_background_color(s_main_window, GColorSunsetOrange);
            break;

        case failed:
            action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_UP, s_up_disabled_bitmap);
            action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_DOWN, s_down_disabled_bitmap);
            action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_SELECT, s_play_bitmap);
            text_layer_set_text(s_main_text_layer, failed_str);
            text_layer_set_text(s_sub_text_layer, "Press Select to Restart");
            window_set_background_color(s_main_window, GColorRed);
            break;
    }
}

// Button Handlers
static void select_button_handler(ClickRecognizerRef recognizer, void *context) {
    switch (s_state) {
        case resumed:
            s_state = paused;
            s_pause_handler();
            break;
        case paused:
            s_state = resumed;
            s_resume_handler();
            break;
        case calibrating:
            s_state = paused;
            s_calibrate_end_handler();
            break;
        case failed:
            s_state = resumed;
            s_resume_handler();
            break;
    }

    set_status_display();
}

void window_main_set_failed() {
    s_state = failed;
    set_status_display();
}

static void select_double_button_handler(ClickRecognizerRef recognizer, void *context) {
    if (s_state == paused) {
        s_state = calibrating;
        s_calibrate_start_handler();
        set_status_display();
    }
}

static void up_button_handler(ClickRecognizerRef recognizer, void *context) {
    s_up_handler();
}

static void down_button_handler(ClickRecognizerRef recognizer, void *context) {
    s_down_handler();
}

static void action_click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) select_button_handler);
    window_multi_click_subscribe(BUTTON_ID_SELECT, 2, 2, 0, false, (ClickHandler) select_double_button_handler);
    window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) up_button_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) down_button_handler);
}

static void window_main_load(Window *window) {
    Layer *root_layer = window_get_root_layer(window);
    GRect root_bounds = layer_get_bounds(root_layer);

    // Text Layers
    const GEdgeInsets main_text_inset = {.top = 28, .right = ACTION_BAR_WIDTH};
    s_main_text_layer = text_layer_create(grect_inset(root_bounds, main_text_inset));
    text_layer_set_text(s_main_text_layer, paused_str);
    text_layer_set_background_color(s_main_text_layer, GColorClear);
    text_layer_set_text_alignment(s_main_text_layer, GTextAlignmentCenter);
    text_layer_set_font(s_main_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    layer_add_child(root_layer, text_layer_get_layer(s_main_text_layer));

    const GEdgeInsets sub_text_inset = {.top = root_bounds.size.h - 43, .left=5, .right = ACTION_BAR_WIDTH + 5};
    s_sub_text_layer = text_layer_create(grect_inset(root_bounds, sub_text_inset));
    text_layer_set_text(s_sub_text_layer, "Double Press Select to Calibrate");
    text_layer_set_background_color(s_sub_text_layer, GColorClear);
    text_layer_set_text_alignment(s_sub_text_layer, GTextAlignmentCenter);
    text_layer_set_font(s_sub_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    layer_add_child(root_layer, text_layer_get_layer(s_sub_text_layer));

    // Status Bar Layer
    s_status_bar_layer = status_bar_layer_create();
    layer_add_child(root_layer, status_bar_layer_get_layer(s_status_bar_layer));

    // Action Bar Layer
    s_play_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PLAY);
    s_pause_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PAUSE);
    s_up_bitmap = gbitmap_create_with_resource(RESOURCE_ID_UP);
    s_down_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DOWN);
    s_up_disabled_bitmap = gbitmap_create_with_resource(RESOURCE_ID_UP_DISABLED);
    s_down_disabled_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DOWN_DISABLED);
    s_stop_bitmap = gbitmap_create_with_resource(RESOURCE_ID_STOP);

    s_action_bar_layer = action_bar_layer_create();
    action_bar_layer_set_click_config_provider(s_action_bar_layer, action_click_config_provider);
    set_status_display();

    action_bar_layer_add_to_window(s_action_bar_layer, window);
}

static void window_main_unload(Window *window) {
    text_layer_destroy(s_main_text_layer);
    text_layer_destroy(s_sub_text_layer);
    action_bar_layer_destroy(s_action_bar_layer);
    status_bar_layer_destroy(s_status_bar_layer);
    gbitmap_destroy(s_play_bitmap);
    gbitmap_destroy(s_up_bitmap);
    gbitmap_destroy(s_down_bitmap);
    gbitmap_destroy(s_stop_bitmap);
    gbitmap_destroy(s_up_disabled_bitmap);
    gbitmap_destroy(s_down_disabled_bitmap);
    window_destroy(window);
}

void window_main_push(ButtonHandler pause_handler,
                      ButtonHandler resume_handler,
                      ButtonHandler up_handler,
                      ButtonHandler down_handler,
                      ButtonHandler calibrate_start_handler,
                      ButtonHandler calibrate_end_handler) {
    s_state = paused;
    s_pause_handler = pause_handler;
    s_resume_handler = resume_handler;
    s_up_handler = up_handler;
    s_down_handler = down_handler;
    s_calibrate_start_handler = calibrate_start_handler;
    s_calibrate_end_handler = calibrate_end_handler;

    s_main_window = window_create();


    window_set_window_handlers(s_main_window, (WindowHandlers) {
            .load = window_main_load,
            .unload = window_main_unload,
    });

    window_stack_push(s_main_window, true);
}
