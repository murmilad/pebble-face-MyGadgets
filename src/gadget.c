#include <pebble.h>
#include "gadget.h"

#define INTERVAL_MINUTES 3

#define TRAY_IMAGES_COUNT 2

#define KEY_GADGET_TYPE 0
#define KEY_GADGET_VALUE 1
#define KEY_GADGET_COMMAND 2
#define KEY_GADGET_TIME 3
#define KEY_FIGURE_COUNT 4
#define KEY_FIGURE_NUMBER 5
#define KEY_FIGURES_SENT 6
#define KEY_STATISTICS_COUNT 7
#define KEY_GADGET_NAME 8
#define KEY_GADGET_NUMBER 9
#define KEY_GADGET_COUNT 10
#define KEY_GADGET_STRING 11

typedef enum {
  ReceiveTypeGadget = 1,
  ReceiveTypeFigure = 2,
  ReceiveTypeStatistics = 3
} ReceiveType;

typedef struct {
  uint8_t type;
  time_t time;
  int16_t value;
  char string[32];
} Figure;

typedef struct {
  char title[120];
} Gadget;

static Figure *s_figures_array;
static Figure *s_statistics_array;
static Gadget *s_gadgets_array;

static BitmapLayer *s_tray_layer;
static BitmapLayer *s_data_layer;
static GBitmap *s_tray_bitmap[TRAY_IMAGES_COUNT];

static TextLayer *s_data_text_layer;

static bool s_figures_received = false;
static bool s_statiscics_received = false;
static bool s_gadgets_received = false;

static bool s_is_gadget = false;
static bool s_is_figure = false;
static bool s_is_statistics = false;

static uint8_t s_figure_count = 0;
static GFont s_data_font;

static void send_command(char * command) {
  DictionaryIterator* dictionaryIterator = NULL;
  app_message_outbox_begin (&dictionaryIterator);
  dict_write_cstring (dictionaryIterator, KEY_GADGET_COMMAND, command);
  dict_write_end (dictionaryIterator);
  app_message_outbox_send ();
}

void tick_gadget() {

  static uint8_t s_interval_minutes = 0;
  
  if (s_interval_minutes == 0 || s_interval_minutes == INTERVAL_MINUTES) {
    if (s_gadgets_received){
      send_command("send_figures");
    } else {
      send_command("send_gadgets");
    }
    s_interval_minutes = 1;
  }
  
  s_interval_minutes++;
}

void update_gadget() {
  
  if (s_figures_received){
    if (s_figure_count > 0) {
        bitmap_layer_set_bitmap(s_tray_layer, s_tray_bitmap[1]);
        
        static char title_str[512];
    
        strcpy(title_str, "");
    
        for (uint8_t i = 0; i < s_figure_count ; i++){
          char figure_str[64];
          snprintf(figure_str, sizeof(figure_str), "%s : %s\n", s_gadgets_array[s_figures_array[i].type].title, s_figures_array[i].string);
          strcat(title_str, figure_str);
          APP_LOG(APP_LOG_LEVEL_INFO, " title %s", title_str);
        }
    
        APP_LOG(APP_LOG_LEVEL_INFO, " title %s", title_str);
    
        text_layer_set_text(s_data_text_layer, title_str);
      } else {
        s_figures_received = false;
        bitmap_layer_set_bitmap(s_tray_layer, s_tray_bitmap[0]);
      }
    } else {
    bitmap_layer_set_bitmap(s_tray_layer, s_tray_bitmap[0]);
  }
}

void load_gadget(Window *window) {
    // Create GBitmap, then set to created BitmapLayer
  s_tray_bitmap[0] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_TRAY_LOAD);
  
  s_tray_bitmap[1] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_TRAY_CLEAN);

  s_tray_layer = bitmap_layer_create(GRect(83, 25, 61, 61));
  bitmap_layer_set_bitmap(s_tray_layer, s_tray_bitmap[0]);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_tray_layer));

  s_data_layer = bitmap_layer_create(GRect(0, 86, 144, 82));
  bitmap_layer_set_bitmap(s_data_layer, gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DATA_FRAME));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_data_layer));

  s_data_text_layer = text_layer_create(GRect(10, 103, 130, 62));
  text_layer_set_background_color(s_data_text_layer, GColorBlack);
  text_layer_set_text_color(s_data_text_layer, GColorClear);

  s_data_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GEORGIA14));
  text_layer_set_font(s_data_text_layer, s_data_font);

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_data_text_layer));
}

void unload_gadget(){
    for(int i = 0; i < TRAY_IMAGES_COUNT; i++) {
      gbitmap_destroy(s_tray_bitmap[i]);
    }

    bitmap_layer_destroy(s_tray_layer);
    bitmap_layer_destroy(s_data_layer);
    free(s_figures_array);
    free(s_gadgets_array);
    free(s_statistics_array);
    text_layer_destroy(s_data_text_layer);

}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Read first item
  APP_LOG(APP_LOG_LEVEL_INFO, "read");
  Tuple *t = dict_read_first(iterator);
  APP_LOG(APP_LOG_LEVEL_INFO, "readed");

  Figure figure;
  Gadget gadget;

  uint8_t figure_number = 0;
  uint8_t gadget_number = 0;

  static uint8_t figure_count = 0;

  // For all items
  while(t != NULL) {
    // Which key was received?
     APP_LOG(APP_LOG_LEVEL_INFO, "Key %d",(int) t->key);
    switch(t->key) {
      case KEY_GADGET_COUNT:
        APP_LOG(APP_LOG_LEVEL_INFO, "Gadget count %d", t->value->uint8);
        s_gadgets_received = false;

        free(s_gadgets_array);
        s_gadgets_array = (Gadget*)malloc(t->value->uint8 * sizeof(Gadget));
        send_command("send_next_gadget");
        break;
      case KEY_FIGURE_COUNT:
        s_figure_count = 0;
        figure_count = t->value->uint8;
        APP_LOG(APP_LOG_LEVEL_INFO, "Figure count %d", t->value->uint8);
        s_figures_received = false;

        free(s_figures_array);
        s_figures_array = (Figure*)malloc(t->value->uint8 * sizeof(Figure));
        send_command("send_next_figure");
        break;
      case KEY_STATISTICS_COUNT:
        APP_LOG(APP_LOG_LEVEL_INFO, "Station count %d", t->value->uint8);
        s_statiscics_received = false;

        free(s_statistics_array);
        s_statistics_array = (Figure*)malloc(t->value->uint8 * sizeof(Figure));
        send_command("send_next_statistics");
        break;
      case KEY_GADGET_TYPE:
        s_is_figure = true;
        APP_LOG(APP_LOG_LEVEL_INFO, "Gadget type %d", t->value->uint8);
        figure.type = t->value->uint8;
        break;
      case KEY_GADGET_VALUE:
        APP_LOG(APP_LOG_LEVEL_INFO, "Gadget value %d", t->value->int16);
        figure.value = t->value->int16;
        break;
      case KEY_GADGET_NAME:
        APP_LOG(APP_LOG_LEVEL_INFO, "Gadget name %s", t->value->cstring);
        snprintf(gadget.title, sizeof(gadget.title), "%s", t->value->cstring);
        break;
      case KEY_GADGET_NUMBER:
        s_is_gadget = true;
        APP_LOG(APP_LOG_LEVEL_INFO, "Gadget number %d", t->value->uint8);
        gadget_number = t->value->uint8;
        break;
      case KEY_FIGURE_NUMBER:
        s_is_statistics = true;
        APP_LOG(APP_LOG_LEVEL_INFO, "Figure number %d", t->value->uint8);
        figure_number = t->value->uint8;
        break;
      case KEY_GADGET_STRING:
        APP_LOG(APP_LOG_LEVEL_INFO, "Gadget string %s", t->value->cstring);
        snprintf(figure.string, sizeof(figure.string), "%s", t->value->cstring);
        break;
      case KEY_FIGURES_SENT:
        switch((ReceiveType) t->value->uint8) {
          case ReceiveTypeGadget:
            s_gadgets_received = true;
            s_is_gadget = false;
            send_command("send_figures");
            break;
          case ReceiveTypeFigure:
            s_figures_received = true;
            update_gadget();
            s_is_figure = false;
            break;
          case ReceiveTypeStatistics:
            s_statiscics_received = true;
            s_is_statistics = false;
            break;
          default:
            printf("Unexpected value for state_now: %d", t->value->uint8);
            break;
          }
        break;
        default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
        break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }

  if (s_is_figure) {
    printf("Add Figure: %d", figure.type);
    s_figures_array[figure.type] = figure;
    s_figure_count = figure_count;

    send_command("send_next_figure");
  }
  if (s_is_statistics) {
    s_statistics_array[figure_number] = figure;
    send_command("send_next_statistics");
  }
  if (s_is_gadget) {
    s_gadgets_array[gadget_number] = gadget;
    send_command("send_next_gadget");
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

void init_gadget(){
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  send_command("send_gadgets");

}