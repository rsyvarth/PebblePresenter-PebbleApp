//Pebble Presenter
//By Robert Syvarth
//Based off of https://github.com/Neal/pebble-vlc-remote

#include <pebble.h>

static Window *window;

static ActionBarLayer *action_bar;

static GBitmap *action_icon_vol_up;
static GBitmap *action_icon_vol_down;
static GBitmap *action_icon_play;
static GBitmap *action_icon_pause;

static TextLayer *title_layer;
static TextLayer *slide_timer_layer;
// static TextLayer *status_layer;
static TextLayer *auth_text_layer;
static TextLayer *auth_layer;

static char title[30] = "Pebble Presenter";
static char timer_text[20] = "0";
// static char status[8] = "Unknown";
static char auth_text[] = "Auth Code:";
static char auth[5] = "   ";
const  uint32_t clock_timeout_const = 1000;
//static int clock_timeout = 1;
static int clock_time = 90;
static char timerText[30];



enum {
  KEY_REQUEST,
  KEY_TITLE,
  KEY_STATUS,
  KEY_AUTH,
};

static void out_sent_handler(DictionaryIterator *sent, void *context) {
  // outgoing message was delivered
}

void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Failed to send AppMessage to Pebble");
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *title_tuple = dict_find(iter, KEY_TITLE);
  //Tuple *status_tuple = dict_find(iter, KEY_STATUS);
  Tuple *auth_tuple = dict_find(iter, KEY_AUTH);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "incoming message from Pebble");

  if (title_tuple) {
    strncpy(title, title_tuple->value->cstring, sizeof(title));
    text_layer_set_text(title_layer, title);
  }
  // if (status_tuple) {
  //   strncpy(status, status_tuple->value->cstring, sizeof(status));
  //   //text_layer_set_text(status_layer, status);
  //   if (strcmp(status, "Playing") == 0) {
  //     action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_pause);
  //   } else {
  //     action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_play);
  //   }
  // }
  if (auth_tuple) {
    strncpy(auth, auth_tuple->value->cstring, sizeof(auth));
    text_layer_set_text(auth_layer, auth);
  }
}

void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "incoming message from Pebble dropped");
}

void send_request(char *request) {
  Tuplet request_tuple = TupletCString(KEY_REQUEST, request);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &request_tuple);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  send_request("next");
  vibes_short_pulse();
}

static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  send_request("prev");
  vibes_double_pulse();
}

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  send_request("refresh");
  vibes_long_pulse();
}

// static void up_long_click_handler(ClickRecognizerRef recognizer, void *context) {
//   send_request("vol_max");
// }

// static void down_long_click_handler(ClickRecognizerRef recognizer, void *context) {
//   send_request("vol_min");
// }

static void click_config_provider(void *context) {
  const uint16_t repeat_interval_ms = 1000;
  window_single_repeating_click_subscribe(BUTTON_ID_UP, repeat_interval_ms, up_single_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, repeat_interval_ms, down_single_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_SELECT, repeat_interval_ms, select_single_click_handler);
  // window_long_click_subscribe(BUTTON_ID_UP, 700, up_long_click_handler, NULL);
  // window_long_click_subscribe(BUTTON_ID_DOWN, 700, down_long_click_handler, NULL);
}



//-----------------------------------
// Hack to include the iota functions
//-----------------------------------

/* reverse:  reverse string s in place */
void reverse(char s[]){
     int i, j;
     char c;
 
     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
}

void itoa(int n, char s[]){
     int i, sign;
 
     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
}

//-----------------------------------
// END: Hack to include the iota functions
//-----------------------------------

static void timer_callback(void *context) { 
  char tmp[10];

  clock_time = clock_time - 1;

  if( clock_time < 0 ) {
    clock_time = 90;
  }

  int min = clock_time % 60;
  int sec = clock_time - (min * 60);
  
  itoa(min, timerText);
  strcat(timerText, ":");
  strcat(timerText, sec);

  // APP_LOG(APP_LOG_LEVEL_DEBUG, "timer hit");
  // APP_LOG(APP_LOG_LEVEL_DEBUG, timerText);

  text_layer_set_text(slide_timer_layer, timerText);
  
  app_timer_register(clock_timeout_const, timer_callback, NULL);
}

static void window_load(Window *window) {
  action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(action_bar, window);
  action_bar_layer_set_click_config_provider(action_bar, click_config_provider);

  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_vol_up);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_vol_down);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_play);

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  title_layer = text_layer_create((GRect) { .origin = { 5, 0 }, .size = { bounds.size.w - 28, 52 } });
  text_layer_set_text_color(title_layer, GColorBlack);
  text_layer_set_background_color(title_layer, GColorClear);
  text_layer_set_font(title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

  slide_timer_layer = text_layer_create((GRect) { .origin = { 5, 54 }, .size = bounds.size });
  text_layer_set_text_color(slide_timer_layer, GColorBlack);
  text_layer_set_background_color(slide_timer_layer, GColorClear);
  text_layer_set_font(slide_timer_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));

  // status_layer = text_layer_create((GRect) { .origin = { 10, 54 + 14 }, .size = bounds.size });
  // text_layer_set_text_color(status_layer, GColorBlack);
  // text_layer_set_background_color(status_layer, GColorClear);
  // text_layer_set_font(status_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));

  auth_text_layer = text_layer_create((GRect) { .origin = { 5, 100 }, .size = bounds.size });
  text_layer_set_text_color(auth_text_layer, GColorBlack);
  text_layer_set_background_color(auth_text_layer, GColorClear);
  text_layer_set_font(auth_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));

  auth_layer = text_layer_create((GRect) { .origin = { 10, 100 + 14 }, .size = bounds.size });
  text_layer_set_text_color(auth_layer, GColorBlack);
  text_layer_set_background_color(auth_layer, GColorClear);
  text_layer_set_font(auth_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));

  layer_add_child(window_layer, text_layer_get_layer(title_layer));
  layer_add_child(window_layer, text_layer_get_layer(slide_timer_layer));
  // layer_add_child(window_layer, text_layer_get_layer(status_layer));
  layer_add_child(window_layer, text_layer_get_layer(auth_text_layer));
  layer_add_child(window_layer, text_layer_get_layer(auth_layer));
}

static void window_unload(Window *window) {
  gbitmap_destroy(action_icon_vol_up);
  gbitmap_destroy(action_icon_vol_down);
  gbitmap_destroy(action_icon_play);
  gbitmap_destroy(action_icon_pause);
  text_layer_destroy(title_layer);
  text_layer_destroy(slide_timer_layer);
  // text_layer_destroy(status_layer);
  text_layer_destroy(auth_text_layer);
  text_layer_destroy(auth_layer);
  action_bar_layer_destroy(action_bar);
}

static void app_message_init(void) {
  app_message_open(96 /* inbound_size */, 64 /* outbound_size */);
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_register_outbox_failed(out_failed_handler);
}

static void init(void) {
  action_icon_vol_up = gbitmap_create_with_resource(RESOURCE_ID_ICON_VOLUME_UP);
  action_icon_vol_down = gbitmap_create_with_resource(RESOURCE_ID_ICON_VOLUME_DOWN);
  action_icon_play = gbitmap_create_with_resource(RESOURCE_ID_ICON_PLAY);
  action_icon_pause = gbitmap_create_with_resource(RESOURCE_ID_ICON_PAUSE);

  app_message_init();

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true /* animated */);

  text_layer_set_text(title_layer, title);
  text_layer_set_text(slide_timer_layer, timer_text);
  // text_layer_set_text(status_layer, status);
  text_layer_set_text(auth_text_layer, auth_text);
  text_layer_set_text(auth_layer, auth);

  app_timer_register(clock_timeout_const, timer_callback, NULL);

  send_request("refresh");
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
