#include <pebble.h>

#include "entry.h"

static Window *window;
static TextLayer *symbol_layer;
static TextLayer *price_layer;
static char symbol[5];
static char price[10];
static int total_slide;
static int current_slide;

enum {
  QUOTE_KEY_SYMBOL = 0x0,
  QUOTE_KEY_PRICE = 0x1,
  QUOTE_KEY_FETCH = 0x2,
};

static void set_symbol_msg(char *symbol) {
  Tuplet symbol_tuple = TupletCString(QUOTE_KEY_SYMBOL, symbol);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {return;}

  dict_write_tuplet(iter, &symbol_tuple);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void fetch_msg(void) {//sends out current 
  Tuplet fetch_tuple = TupletInteger(QUOTE_KEY_FETCH, 1);
  Tuplet price_tuple = TupletInteger(QUOTE_KEY_PRICE, 1);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {return;}

  dict_write_tuplet(iter, &fetch_tuple);
  dict_write_tuplet(iter, &price_tuple);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // refresh
  text_layer_set_font(symbol_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text(price_layer, "Retrieving auth code");
  fetch_msg();
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  // refresh
  entry_get_name(symbol, set_symbol_msg);
  text_layer_set_text(symbol_layer, symbol);
  text_layer_set_text(price_layer, "Loading...");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  // decrement current_slide, ensure wraparound, send to phone instructions to go back one slide, 
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  // increment current_slide, ensure wraparound, send to phone instructions to go to next slide
}

static void click_config_provider(void *context) {
  const uint16_t repeat_interval_ms = 100;
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 0, select_long_click_handler, NULL);
  window_single_repeating_click_subscribe(BUTTON_ID_UP, repeat_interval_ms, up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, repeat_interval_ms, down_click_handler);
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *symbol_tuple = dict_find(iter, QUOTE_KEY_SYMBOL);
  Tuple *price_tuple = dict_find(iter, QUOTE_KEY_PRICE);

  if (symbol_tuple) {
    strncpy(symbol, symbol_tuple->value->cstring, 5);
    text_layer_set_text(symbol_layer, symbol);
  }
  if (price_tuple) {
    strncpy(price, price_tuple->value->cstring, 10);
    text_layer_set_text(price_layer, price);
  }
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Dropped!");
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Failed to Send!");
}

static void app_message_init(void) {
  // Register message handlers
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_failed(out_failed_handler);
  // Init buffers
  app_message_open(64, 64);
  fetch_msg();
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  symbol_layer = text_layer_create(
      (GRect) { .origin = { 0, 20 }, .size = { bounds.size.w, 50 } });
  text_layer_set_text(symbol_layer, "Pebble Presenter"); 
  text_layer_set_text_alignment(symbol_layer, GTextAlignmentCenter); 
  text_layer_set_font(symbol_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(symbol_layer));

  price_layer = text_layer_create(
      (GRect) { .origin = { 0, 75 }, .size = { bounds.size.w, 50 } });
  text_layer_set_text(price_layer, "Retrieving auth code");
  text_layer_set_text_alignment(price_layer, GTextAlignmentCenter);
  text_layer_set_font(price_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  layer_add_child(window_layer, text_layer_get_layer(price_layer));

  //TODO: write a function that sends an empty id, waits for return
  //char[] auth = return with auth code
  //write auth to screen

  text_layer_set_text(price_layer, "Auth code: ####");

  //here, request total_slide?
  fetch_msg();
}

static void window_unload(Window *window) {
  text_layer_destroy(symbol_layer);
  text_layer_destroy(price_layer);
}

static void init(void) {
  window = window_create();
  app_message_init();
  //char entry_title[] = "Enter Symbol";
  //entry_init(entry_title);
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {window_destroy(window);}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
