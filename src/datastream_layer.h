#ifndef DATASTREAM_LAYER_H
#define DATASTREAM_LAYER_H

#include <pebble.h>

#define TEXT_WIDTH 48
#define ICON_WIDTH 32
#define GRAPH_WIDTH 64
#define LAYER_HEIGHT 42

// Key values for AppMessage Dictionary
enum {
  MSG_TYPE_KEY = 0,
  CHART_HEIGHT_KEY = 1,
  CHANNEL_ID_KEY = 10,
  FIELD_NUM_KEY = 11,
  DATA_KEY = 12,
  VALUE_KEY = 13,
  VALUE_MIN_KEY = 14,
  VALUE_MAX_KEY = 15,
  API_KEY_KEY = 500,
  GRAPH_WIDTH_KEY = 501
};

typedef struct {
  Layer* graph;
  BitmapLayer* icon;
  TextLayer* text;
  TextLayer* textMin;
  TextLayer* textMax;
  Layer* layer;
  uint8_t resource;
  char value[8];
  char valueMin[8];
  char valueMax[8];
  int32_t channelId;
  int32_t fieldNum;
  char apiKey[32];
  int8_t pixel[GRAPH_WIDTH];
} DataStreamLayer;


DataStreamLayer* datastream_layer_create(int32_t channelId, int32_t fieldNum, const char* apiKey, GPoint pos);
void datastream_layer_destroy(DataStreamLayer* layer);
void datastream_layer_set_icon(DataStreamLayer* layer, uint8_t resource);
void datastream_layer_set_text(DataStreamLayer* layer, const char* value, const char* valueMin, const char* valueMax);
void datastream_layer_set_graph_dict(DataStreamLayer* layer, DictionaryIterator* received);
void datastream_layer_set_graph(DataStreamLayer* layer, const char* pGraph);
void datastream_layer_request_data(DataStreamLayer* layer);

#endif
