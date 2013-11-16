#include <pebble.h>

static Window *window;
static TextLayer *slide_layer;
static TextLayer *auth_layer;
static char auth[5];
static int total_slide =0;
static int current_slide=0;

void display_Slides(void);//void reverse(char);void itoa(int, char);

enum {
  AUTH_KEY = 0x0,
  SLIDE_KEY = 0x1,
};

static void set_auth(char *auth) {//sends auth request to phone
  auth = "refr";
  Tuplet auth_tuple = TupletCString(AUTH_KEY, auth);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  if (iter == NULL) {return;}

  dict_write_tuplet(iter, &auth_tuple);
  dict_write_end(iter);
  app_message_outbox_send();
}

static void set_slide(int command) {//sends slide commands to phone; 0 for total slide, 1 for next slide, -1 for previous slide
  Tuplet slide_tuple = TupletInteger(SLIDE_KEY, command);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  if (iter == NULL) {return;}

  dict_write_tuplet(iter, &slide_tuple);
  dict_write_end(iter);
  app_message_outbox_send();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) { 
  text_layer_set_text(auth_layer, "Retrieving auth code");
  set_auth("refr");//UMM SOME PROBLEM WITH STRINGSSSS?! RES
  set_slide(0);
  current_slide=1;
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {}//Do we really need this? maybe jump to start of slideshow?

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

void display_Slides(void){
  char cur_slide_str[33]; itoa(current_slide,cur_slide_str);
  char tot_slide_str[33]; itoa(total_slide,tot_slide_str);

  char* displayslide = malloc(strlen("Slide ")+strlen(cur_slide_str)+strlen(tot_slide_str)+2);//JANKY, PROBABLY NEEDS FIXING
  strcpy(displayslide,"Slide "); strcat(displayslide,cur_slide_str); strcat(displayslide,"/"); strcat(displayslide,tot_slide_str);
  displayslide[strlen(displayslide)] = '\0';

  text_layer_set_font(slide_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text(slide_layer, displayslide); //CHARLIE WHAT THE HECK WERE YOU THINKING
  free(displayslide);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  current_slide--; 
  if(current_slide<=0){current_slide=1;} 
  set_slide(-1);

  display_Slides();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  current_slide++; 
  if(current_slide>total_slide){current_slide=total_slide;} 
  set_slide(1);

  display_Slides();
}

//picks which handler to run based on which button was pressed
static void click_config_provider(void *context) {
  const uint16_t repeat_interval_ms = 100;
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 0, select_long_click_handler, NULL);
  window_single_repeating_click_subscribe(BUTTON_ID_UP, repeat_interval_ms, up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, repeat_interval_ms, down_click_handler);
}

static void in_received_handler(DictionaryIterator *iter, void *context) {//recieves symbol and price from phone// 
  Tuple *slide_tuple = dict_find(iter, SLIDE_KEY);
  Tuple *auth_tuple = dict_find(iter, AUTH_KEY);

  if (slide_tuple) {
    if(slide_tuple->value->data > 0){
		total_slide = (int) slide_tuple->value->data;
		current_slide = 1;
	}	//TYPECAST PROBLEMS SHRUG? RES
  }
  if (auth_tuple) {
    strncpy(auth, auth_tuple->value->cstring, 5);

	char* displayauth = malloc(strlen("Auth code: ")+strlen(auth)+2);//JANKY, PROBABLY NEEDS FIXING
	strcpy(displayauth,"Auth code: "); strcat(displayauth,auth);
	displayauth[strlen(displayauth)] = '\0';

	text_layer_set_text(auth_layer, displayauth); //CHARLIE WHAT THE HECK WERE YOU THINKING
	free(displayauth);
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
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  slide_layer = text_layer_create( //CREATING THE "SLIDE #/#" LAYER
      (GRect) { .origin = { 0, 20 }, .size = { bounds.size.w, 50 } });
  text_layer_set_text(slide_layer, "Pebble Presenter"); 
  text_layer_set_text_alignment(slide_layer, GTextAlignmentCenter); 
  text_layer_set_font(slide_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(slide_layer));

  auth_layer = text_layer_create( //CREATING THE "AUTH" LAYER
      (GRect) { .origin = { 0, 75 }, .size = { bounds.size.w, 50 } });
  text_layer_set_text(auth_layer, "Retrieving auth code");
  text_layer_set_text_alignment(auth_layer, GTextAlignmentCenter);
  text_layer_set_font(auth_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  layer_add_child(window_layer, text_layer_get_layer(auth_layer));

  set_auth("refr"); //UM OK FIX THIS, RES
  set_slide(0);
}

static void window_unload(Window *window) {
  text_layer_destroy(slide_layer);
  text_layer_destroy(auth_layer);
}

static void init(void) {
  window = window_create();
  app_message_init();
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
