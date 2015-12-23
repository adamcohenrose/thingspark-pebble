#include <pebble.h>
#include <pebble_fonts.h>

#include "datastream_layer.h"

Window *window;
TextLayer* datetime;

static char time_str[16] = "12:12\0";
static char* const MSG_TYPE_SETTINGS = "settings";
static char* const MSG_TYPE_DATA = "data";

#define DATASTREAM_SLOTS 1
static DataStreamLayer* ds[DATASTREAM_SLOTS];

static DataStreamLayer* find_datastream(int32_t channelId, int32_t fieldNum) {
  if (channelId == 0 || fieldNum == 0) {
    return NULL;
  }

  for (int idx = 0; idx < DATASTREAM_SLOTS; idx++) {
    if (ds[idx]->channelId == channelId && ds[idx]->fieldNum == fieldNum) {
      return ds[idx];
    }
  }

  return NULL;
}

static DataStreamLayer* datastream_layer_next_layer(DataStreamLayer* layer) {
  for (int idx = 0; idx < DATASTREAM_SLOTS; idx++) {
    if (ds[idx] == layer && idx + 1 < DATASTREAM_SLOTS) {
      return ds[idx+1];
    }
  }

  return NULL;
}

const char* extract_msg_string(int key, DictionaryIterator *dict) {
  Tuple* tuple = dict_find(dict, key);
  if (tuple) {
    return tuple->value->cstring;
  }

  return NULL;
}

int32_t extract_msg_int(int key, DictionaryIterator *dict) {
  Tuple* tuple = dict_find(dict, key);
  if (tuple && tuple->type == TUPLE_CSTRING) {
    return atoi(tuple->value->cstring);
  } else if (tuple) {
    return tuple->value->int32;
  }

  return 0;
}

static void add_datastream_layer(int idx, int32_t channelId, int32_t fieldNum, 
                                 const char* apiKey, uint8_t icon) {

  ds[idx] = datastream_layer_create(channelId, fieldNum, apiKey, GPoint(0, (LAYER_HEIGHT + 8)*idx + 25));
  datastream_layer_set_icon(ds[idx], icon);

  layer_add_child(window_get_root_layer(window), ds[idx]->layer);
  datastream_layer_set_text(ds[idx], "N/A", "", "");

  datastream_layer_request_data(ds[idx]);
}

static void create_layers_from_settings() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "setting up layers from saved settings...");
  
  // TODO: allow multiple sparklines
  int idx = 0;
  
  int32_t channelId;
  int32_t fieldNum;
  uint8_t icon;
  char apiKey[32];
  if (!persist_exists(CHANNEL_ID_KEY)) {
    // sample values for demoing the app
    channelId = 9;
    fieldNum = 1;
    icon = RESOURCE_ID_ICON_SUN_CLOUD;
    
  } else {
    channelId = persist_read_int(CHANNEL_ID_KEY);
    fieldNum = persist_read_int(FIELD_NUM_KEY);
    persist_read_string(API_KEY_KEY, apiKey, sizeof(apiKey));
    // TODO: choose icon in settings
    icon = RESOURCE_ID_ICON_SUN_CLOUD;
  }
  
  add_datastream_layer(idx, channelId, fieldNum, apiKey, icon);
}

// Called when a message is received from PebbleKitJS
static void in_received_handler(DictionaryIterator *dict, void *context) {
  int32_t channelId = extract_msg_int(CHANNEL_ID_KEY, dict);
  int32_t fieldNum = extract_msg_int(FIELD_NUM_KEY, dict);

  const char* msgType = extract_msg_string(MSG_TYPE_KEY, dict);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "msg type: %s", msgType);
  if (strcmp(msgType, MSG_TYPE_SETTINGS) == 0) {
    persist_write_int(CHANNEL_ID_KEY, channelId);
    persist_write_int(FIELD_NUM_KEY, fieldNum);
    const char* apiKey = extract_msg_string(API_KEY_KEY, dict);
    if (apiKey != NULL) {
      persist_write_string(API_KEY_KEY, apiKey);
    }

    // delete any existing DataStreamLayers
    for (int idx = 0; idx < DATASTREAM_SLOTS; idx++) {
      if (ds[idx] != NULL) {
        datastream_layer_destroy(ds[idx]);
      }
    }

    // create layers afresh
    // TODO: choose icon in settings
    uint8_t icon = RESOURCE_ID_ICON_SUN_CLOUD;
    add_datastream_layer(0, channelId, fieldNum, apiKey, icon);

  } else if (strcmp(msgType, MSG_TYPE_DATA) == 0) {
    DataStreamLayer* layer = find_datastream(channelId, fieldNum);
    if (layer != NULL) {
      const char* data = extract_msg_string(DATA_KEY, dict);
      const char* value = extract_msg_string(VALUE_KEY, dict);
      const char* valueMin = extract_msg_string(VALUE_MIN_KEY, dict);
      const char* valueMax = extract_msg_string(VALUE_MAX_KEY, dict);

      datastream_layer_set_graph(layer, data);
      datastream_layer_set_text(layer, value, valueMin, valueMax);

      // TODO: not sure why this was here...?
      //datastream_layer_request_data(datastream_layer_next_layer(layer));
    }
  }
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  strftime (time_str, sizeof(time_str), "%R", tick_time);
  text_layer_set_text(datetime, time_str);

  for (int idx = 0; idx < DATASTREAM_SLOTS; idx++) {
    datastream_layer_request_data(ds[idx]);
  }
}

static void window_load(Window *window) {
  datetime = text_layer_create(GRect(80, 0, 64, 20));
  text_layer_set_text_alignment(datetime, GTextAlignmentCenter);
  text_layer_set_font(datetime, fonts_get_system_font (FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_background_color(datetime, GColorWhite);
  text_layer_set_text_color(datetime, GColorCobaltBlue);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(datetime));
  text_layer_set_text(datetime, time_str);
  
  create_layers_from_settings();
}

static void window_unload(Window *window) {
  tick_timer_service_unsubscribe();
  app_message_deregister_callbacks();

  layer_remove_from_parent(text_layer_get_layer(datetime));
  text_layer_destroy(datetime);

  for (int idx = 0; idx < DATASTREAM_SLOTS; idx++) {
    datastream_layer_destroy(ds[idx]);
  }
}

static void init(void) {
  for (int idx = 0; idx < DATASTREAM_SLOTS; idx++) {
    ds[idx] = NULL;
  }

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  window_set_background_color(window, GColorWhite);

  window_stack_push(window, true);

  // Register AppMessage handlers
  app_message_register_inbox_received(in_received_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
}

static void deinit() {
  window_destroy(window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}