#include <pebble.h>
#include "pebble-assist.h"

static Window *window;
static TextLayer *text_layer;
static ActionBarLayer *action_layer;

static char counter_string[4];
static int counter = 0;

static int text_width = 120;
static int text_height = 80;

static int text_offset_width;
static int text_offset_height;

// static int COUNTER_KEY = 100;

static void setup_calculations() {
  text_offset_width = (PEBBLE_WIDTH - ACTION_BAR_WIDTH - text_width) / 2;
  text_offset_height = (PEBBLE_HEIGHT - STATUS_HEIGHT - text_height) / 2;
}

// Updates the counter & redraws the screen
// change [int] amount to change counter by; negative number decrements
static void update_counter_by(int change) {
  // Update the value we store
  counter = counter + change;
  // Keep us bounded to 0-999
  if (0 > counter || counter > 999) {
    counter = 0;
  }

  // Persist the current count
  // persist_write_int(COUNTER_KEY, counter);

  // Update the display
  snprintf(counter_string, 4, "%d", counter);
  text_layer_set_text(text_layer, counter_string);
}

static void increment_handler(ClickRecognizerRef recognizer, void *context) {
  update_counter_by(1);
}

static void decrement_handler(ClickRecognizerRef recognizer, void *context) {
  update_counter_by(-1);
}

static void light_handler(ClickRecognizerRef recognizer, void *context) {
  light_enable_interaction();
}

static void reset_handler(ClickRecognizerRef recognizer, void *context) {
  update_counter_by(1000); // Force an overflow back to zero
}

static void click_config_provider(void *context) {
  // Short Press up (back) is decrement; Short press down (next) is increment
  window_single_click_subscribe(BUTTON_ID_UP, decrement_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, increment_handler);

  // Turn the light on momentarily
  window_single_click_subscribe(BUTTON_ID_SELECT, light_handler);

  // Reset the counter to zero; 1 second trigger for length
  window_long_click_subscribe(BUTTON_ID_SELECT, 1000, reset_handler, NULL);
}

static void setup_action_layer() {
  action_layer = action_bar_layer_create();
  action_bar_layer_add_to_window(action_layer, window);
  // TODO: add icons to action layer
  // action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, &my_icon_previous);
  // action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, &my_icon_next);
  action_bar_layer_set_click_config_provider(action_layer, click_config_provider);
}

static void setup_text_layer() {
  text_layer = text_layer_create((GRect) { .origin = { text_offset_width, text_offset_height }, .size = { text_width, text_height } });
  text_layer_set_font(text_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LIB_MONO_BOLD_SUBSET_62)));
  text_layer_set_text_alignment(text_layer, GTextAlignmentRight);
  text_layer_set_colours(text_layer, GColorBlack, GColorWhite);
  layer_set_clips(text_layer_get_layer(text_layer), false);
  text_layer_add_to_window(text_layer, window);
}

static void window_load(Window *window) {
  setup_action_layer();
  setup_text_layer();

  // Read the counter out (nil value == 0) and increment counter by it
  // Basically sets screen & `counter' to stored value
  // TODO: work out why this alternates from 0 to 512 on close/open of app
  // update_counter_by(persist_read_int(COUNTER_KEY));
  update_counter_by(0);
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
  action_bar_layer_destroy(action_layer);
}

static void init(void) {
  setup_calculations();

  window = window_create();
  window_set_background_color(window, GColorWhite);
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
