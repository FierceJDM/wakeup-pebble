#define SOURCE_BACKGROUND 1
#include <pebble_worker.h>
static char *received = "0";
AppWorkerMessage message = {
  .data0 = 0,
  .data1 = 0,
};


static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), "%H:%M", tick_time);
  if (strcmp(s_buffer ,"03:30") == 0) {
    if (strcmp(received, "0")==0) {
      persist_write_int(1, 0);
    }
    message.data0 = 1;
    persist_write_int(0, 1);
    app_worker_send_message(SOURCE_BACKGROUND, &message);
  } else {
    persist_write_int(0, 0);
  }
}

static void battery_state_handler(BatteryChargeState charge) {
  if ((int)charge.is_plugged == 1) {
    message.data1 = 1;
    received = "1";
    persist_write_int(1, 1);
    app_worker_send_message(SOURCE_BACKGROUND, &message);
  } else {
    message.data1 = 0;
    received = "1";
    persist_write_int(1, 0); //ERROR HERE
    app_worker_send_message(SOURCE_BACKGROUND, &message);
  }
}





static void init() {
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_state_handler);
}


int main(void) {
  init();
  worker_event_loop();
}