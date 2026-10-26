#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static GBitmap *s_bitmap;
static BitmapLayer *s_bitmap_layer;

static int shift;


static void update_time(struct tm *tick_time);

static void load_shift_bg(){
    int hour_offset = -7;
    time_t temp = time(NULL);
    struct tm *tick_time = gmtime(&temp);
    int now_shift = ((tick_time->tm_hour + hour_offset + 24) % 24)/6;

    //APP_LOG(APP_LOG_LEVEL_DEBUG, "UTC Hour %d + %d Buss Hour %d", tick_time->tm_hour, hour_offset, now_shift);
    if (shift != now_shift){
        gbitmap_destroy(s_bitmap);
        switch(now_shift){
            case 0:
                s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ZETA_SHIFT);
                break;
            case 1:
                s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DAWN_GUARD);
                break;
            case 2:
                 s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ALPHA_FLIGHT);
                 break;
            case 3:
                 s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_NIGHT_WATCH);
                 break;
            case 4:
                 s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_OMEGA);
                 break;
        }
        shift = now_shift;
    }
    
}

static void main_window_load(Window *window) {
    // Get information about the Window
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // Create the TextLayer with specific bounds
    //Add bitmap
    s_bitmap_layer = bitmap_layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
    bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
    bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);

    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bitmap_layer));


    s_time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(8, 5), bounds.size.w, 50));

    // Improve the layout to be more like a watchface
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorWhite);

    update_time(NULL);
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

    // Add it as a child layer to the Window's root layer
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

}

static void main_window_unload(Window *window) {
    // Destroy TextLayer
    text_layer_destroy(s_time_layer);
    gbitmap_destroy(s_bitmap);
    bitmap_layer_destroy(s_bitmap_layer);

}

static void update_time(struct tm *tick_time) {
    if(tick_time==NULL){
        // Get a tm structure
        time_t temp = time(NULL);
        tick_time = localtime(&temp);
    }

    // Write the current hours and minutes into a buffer
    static char s_buffer[8];
    strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

    // Display this time on the TextLayer
    text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    load_shift_bg();
    update_time(tick_time);
}

static void init() {
    shift = -1;
    load_shift_bg();
    // Create main Window element and assign to pointer
    s_main_window = window_create();

    // Set handlers to manage the elements inside the Window
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });

    // Register with TickTimerService
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);


    // Show the Window on the watch, with animated=true
    window_stack_push(s_main_window, true);

}

static void deinit() {
    // Destroy Window
    window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
