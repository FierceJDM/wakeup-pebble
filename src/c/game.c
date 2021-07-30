#include <pebble.h>
#include <time.h>
#define SOURCE_BACKGROUND 1
static Window *s_window;
static TextLayer *s_text_layer;
static ScrollLayer *s_scroll_layer;

static int isPlugged;
static int Vibrate = 0;


static void vibration() {
  APP_LOG(APP_LOG_LEVEL_INFO, "Reached vibration()");
  while (Vibrate==1 && isPlugged == 0) {
    isPlugged = persist_read_int(1);
    APP_LOG(APP_LOG_LEVEL_INFO, "Vibrating");
    vibes_long_pulse();
  }
  APP_LOG(APP_LOG_LEVEL_INFO, "Exiting Vibration Function...");
  persist_write_int(0, 0);
  Vibrate = persist_read_int(0);
  APP_LOG(APP_LOG_LEVEL_INFO, "%d, %d", isPlugged, Vibrate);
}

static void worker_message_handler(uint16_t type, 
                                    AppWorkerMessage *message) {
  if(type == SOURCE_BACKGROUND) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Message Received");
    Vibrate = persist_read_int(0);
    isPlugged = persist_read_int(1);
    APP_LOG(APP_LOG_LEVEL_INFO, "isPlugged = %d", isPlugged);
    if (isPlugged == 0 && Vibrate == 1) {
      APP_LOG(APP_LOG_LEVEL_INFO,  "Vibration Request Sent");
      vibration();
    }
  }

}


static void window_load(Window *window) {
Layer *window_layer = window_get_root_layer(window);
GRect bounds = layer_get_bounds(window_layer);
GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);

// Find the bounds of the scrolling text
GRect shrinking_rect = GRect(0, 0, bounds.size.w, 2000);
char *text = "Loading...                                                                                                      ";
GSize text_size = graphics_text_layout_get_content_size(text, font, 
                shrinking_rect, GTextOverflowModeWordWrap, GTextAlignmentLeft);
GRect text_bounds = bounds;
text_bounds.size.h = text_size.h;

// Create the TextLayer
s_text_layer = text_layer_create(text_bounds);
text_layer_set_overflow_mode(s_text_layer, GTextOverflowModeWordWrap);
text_layer_set_font(s_text_layer, font);

// Create the ScrollLayer
s_scroll_layer = scroll_layer_create(bounds);

// Set the scrolling content size
scroll_layer_set_content_size(s_scroll_layer, text_size);

// Let the ScrollLayer receive click events
scroll_layer_set_click_config_onto_window(s_scroll_layer, window);

// Add the TextLayer as a child of the ScrollLayer
scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_text_layer));

// Add the ScrollLayer as a child of the Window
layer_add_child(window_layer, scroll_layer_get_layer(s_scroll_layer));
}

static void window_unload(Window *window) {
scroll_layer_destroy(s_scroll_layer);
text_layer_destroy(s_text_layer);
}

static void init() {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  
  bool result = app_worker_is_running();
  if (result == true){
    APP_LOG(APP_LOG_LEVEL_INFO, "App worker is running");
  } else {
    AppWorkerResult result = app_worker_launch();
  }
  window_stack_push(s_window, true);

  app_worker_message_subscribe(worker_message_handler);
}

static void deinit() {
  window_destroy(s_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
