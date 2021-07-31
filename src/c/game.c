#include <pebble.h>
#include <time.h>
#define SOURCE_BACKGROUND 1
static Window *s_window;

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
}

static void window_unload(Window *window) {
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
}

static void deinit() {
  window_destroy(s_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
