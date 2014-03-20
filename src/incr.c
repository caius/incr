#include <pebble.h>
#include "pebble-assist.h"

static Window *window;
static TextLayer *text_layer;
static ActionBarLayer *action_layer;

static GBitmap *icon_increment;
static GBitmap *icon_decrement;

static char counter_string[4];
static int counter = 0;

static int text_width = 120;
static int text_height = 80;

static int text_offset_width;
static int text_offset_height;

static int CACHE_COUNTER_KEY = 100;

static void setup_calculations() {
  text_offset_width = (PEBBLE_WIDTH - ACTION_BAR_WIDTH - text_width) / 2;
  text_offset_height = (PEBBLE_HEIGHT - STATUS_HEIGHT - text_height) / 2;
}

// Updates the counter & redraws the screen
static void update_counter_to(int value) {
  counter = value;

  // Keep us bounded to 0-999
  if (0 > counter || counter > 999) {
    counter = 0;
  }

  // Update the display
  snprintf(counter_string, 4, "%d", counter);
  text_layer_set_text(text_layer, counter_string);
}

// change [int] amount to change counter by; negative number decrements
static void update_counter_by(int change) {
  // Update the value we store
  update_counter_to(counter + change);

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
  action_bar_layer_create_in_window(action_layer, window);

  icon_decrement = gbitmap_create_with_resource(RESOURCE_ID_ICON_DECREMENT);
  action_bar_layer_set_icon(action_layer, BUTTON_ID_UP, icon_decrement);

  icon_increment = gbitmap_create_with_resource(RESOURCE_ID_ICON_INCREMENT);
  action_bar_layer_set_icon(action_layer, BUTTON_ID_DOWN, icon_increment);

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
  update_counter_to(persist_read_int(CACHE_COUNTER_KEY));
}

static void window_unload(Window *window) {
  // Persist the current count
  persist_write_int(CACHE_COUNTER_KEY, counter);

  text_layer_destroy_safe(text_layer);
  action_bar_layer_destroy_safe(action_layer);
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
  window_destroy_safe(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
