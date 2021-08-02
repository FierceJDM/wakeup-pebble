#include <pebble.h>
#define SOURCE_BACKGROUND 1
static Window *s_window;
static TextLayer *s_text_layer;
static Layer *window_layer;
static int isPlugged;
static int Vibrate = 0;
static char *text = "Wake-Up time = 03:30\nWatch is not plugged";

static void update_text() {
  if (isPlugged == 0 && Vibrate == 0) {
    text = "Wake-Up time = 03:30\nWatch is not plugged";
  } else if (isPlugged == 1 && Vibrate == 0) {
    text = "Wake-Up time = 03:30\nWatch is plugged";
  }
  text_layer_set_text(s_text_layer, text);
  layer_mark_dirty(window_layer);
}

static void vibration() {
  while (Vibrate==1 && isPlugged == 0) {
    isPlugged = persist_read_int(1);
    APP_LOG(APP_LOG_LEVEL_INFO, "Vibrating");
    vibes_long_pulse();
  }
  APP_LOG(APP_LOG_LEVEL_INFO, "Exiting Vibration Function...");
  persist_write_int(0, 0);
  update_text();
  Vibrate = persist_read_int(0);
}

static void worker_message_handler(uint16_t type, 
                                    AppWorkerMessage *message) {
  if(type == SOURCE_BACKGROUND) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Message Received");
    Vibrate = persist_read_int(0);
    isPlugged = persist_read_int(1);
    update_text();
    if (isPlugged == 0 && Vibrate == 1) {
      APP_LOG(APP_LOG_LEVEL_INFO,  "Vibration Request Sent");
      vibration();
    }
  }

}


static void window_load(Window *window) {
  window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  s_text_layer = text_layer_create(bounds);
  text_layer_set_text_color(s_text_layer, GColorWhite);
  text_layer_set_background_color(s_text_layer, GColorBlack);
  text_layer_set_overflow_mode(s_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  text_layer_set_text(s_text_layer, text);
  
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void window_unload(Window *window) {
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
    APP_LOG(APP_LOG_LEVEL_INFO, "App worker is already running");
  } else {
    AppWorkerResult result = app_worker_launch();
  }
  window_stack_push(s_window, true);
  app_worker_message_subscribe(worker_message_handler);
  APP_LOG(APP_LOG_LEVEL_INFO, "Subscribed !");
}

static void deinit() {
  window_destroy(s_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
