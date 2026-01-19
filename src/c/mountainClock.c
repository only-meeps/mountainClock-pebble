#include <pebble.h>
static Window *s_window;
static TextLayer *s_text_layer;
static TextLayer *s_date_text_layer;
static TextLayer *s_climbed_layer;
static char s_buffer[32];
static char s_date_buffer[16];
static char s_climbed_buffer[8];
static GFont s_generic_font;
static GFont s_generic_small_font;
static Layer *s_line_layer;
static bool climbingUp = true;
static double percent_climbed = 0.5;

static GPoint GPointAdd(GPoint point, GPoint addedPoint)
{
  return GPoint(point.x + addedPoint.x, point.y + addedPoint.y);
}
static GPoint GPointSub(GPoint point, GPoint subPoint)
{
  return GPoint(point.x - subPoint.x, point.y - subPoint.y);
}
static void graphics_update_proc(Layer *layer, GContext *ctx) {
  double scale_factor = 10;
  GRect bounds = layer_get_bounds(layer);
  GPoint start_point1 = GPoint(0, bounds.size.h);
  GPoint end_point1 = GPoint(bounds.size.w / 2, 20);
  GPoint start_point2 = GPoint(bounds.size.w, bounds.size.h);
  GPoint end_point2 = GPoint(bounds.size.w / 2, 20);
  GPoint start_middle = GPoint(bounds.size.w / 2, bounds.size.h);
  GPoint end_middle = GPoint(bounds.size.w / 2, 20);
  GPoint center = GPoint(0,0);
  if(climbingUp)
  {
    center = GPoint(start_point1.x + (end_point1.x - start_point1.x) * percent_climbed, start_point1.y + (end_point1.y - start_point1.y) * percent_climbed);
  }
  else
  {
    center = GPoint(start_point2.x + (end_point2.x - start_point2.x) * percent_climbed, start_point2.y + (end_point2.y - start_point2.y) * percent_climbed);
  }

  double clamped_y = (bounds.size.h - center.y) - 30;
  if(clamped_y < 0)
  {
    clamped_y = 0;
  }
  GPoint offset = GPoint((bounds.size.w - center.x)- bounds.size.w / 2, clamped_y);
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "offset x: %i offset y: %i", offset.x, offset.y);
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "center x: %i center y: %i", center.x, offset.y);
  start_point1 = GPoint(start_point1.x + offset.x, start_point1.y + offset.y);
  end_point1 = GPoint(end_point1.x + offset.x, end_point1.y + offset.y);
  start_point2 = GPoint(start_point2.x + offset.x, start_point2.y + offset.y);
  end_point2 = GPoint(end_point2.x + offset.x - 1, end_point2.y + offset.y);
  start_middle = GPoint(start_middle.x + offset.x, start_middle.y + offset.y);
  end_middle = GPoint(end_middle.x + offset.x, end_middle.y + offset.y);
  GPoint DarkPoints[] = {
    start_point1,
    end_point1,  
    start_middle 
  };
  GPathInfo DarkInfo = {
    .num_points = 3,
    .points = DarkPoints
  };
  GPoint LightPoints[] = {
    start_point2,
    end_point2,  
    {start_middle.x - 1, start_middle.y}
  };
  GPathInfo LightInfo = {
    .num_points = 3,
    .points = LightPoints
  };
  GPoint pointing_spot = GPoint(0,0);
  GPoint Pointers[3];
  if(climbingUp)
  {
    pointing_spot = GPoint(start_point1.x + (end_point1.x - start_point1.x) * percent_climbed, start_point1.y + (end_point1.y - start_point1.y) * percent_climbed);
    Pointers[0] = GPoint(10, 60);
    Pointers[1] = GPoint(80, 60);
    Pointers[2] = pointing_spot;
  }
  else
  {
    pointing_spot = GPoint(start_point2.x + (end_point2.x - start_point2.x) * percent_climbed, start_point2.y + (end_point1.y - start_point2.y) * percent_climbed);
    Pointers[0] = GPoint(bounds.size.w - 10, 60);
    Pointers[1] = GPoint(bounds.size.w - 80, 60);
    Pointers[2] = pointing_spot;
  }
 
  GPathInfo PointerInfo = {
    .num_points = 3,
    .points = Pointers
  };
  GPath *s_dark_shade_path = gpath_create(&DarkInfo);
  GPath *s_light_shade_path = gpath_create(&LightInfo);
  GPath *s_pointer_path = gpath_create(&PointerInfo);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 2);
  gpath_draw_outline_open(ctx, s_pointer_path);
  //graphics_draw_line(ctx, start_middle, end_middle);
  //graphics_draw_line(ctx, start_point2, end_point2);
  //graphics_draw_line(ctx, start_point1, end_point1);
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorGreen, GColorBlack));
  gpath_draw_filled(ctx, s_dark_shade_path);
  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorMintGreen, GColorWhite));
  gpath_draw_filled(ctx, s_light_shade_path);
  gpath_destroy(s_pointer_path);
  gpath_destroy(s_dark_shade_path);
  gpath_destroy(s_light_shade_path);
}
static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorElectricBlue);
  GRect bounds = layer_get_bounds(window_layer);
  s_generic_font = fonts_get_system_font(FONT_KEY_LECO_28_LIGHT_NUMBERS);
  s_text_layer = text_layer_create(GRect(0, bounds.size.h - 65, 80, 30));
  s_generic_small_font = fonts_get_system_font(FONT_KEY_GOTHIC_24);
  s_climbed_layer = text_layer_create(GRect(10, 60, 70, 30));
  s_date_text_layer = text_layer_create(GRect(0, bounds.size.h - 30, 80, 30));
  text_layer_set_font(s_climbed_layer, s_generic_small_font);
  text_layer_set_font(s_text_layer, s_generic_font);
  text_layer_set_font(s_date_text_layer, s_generic_small_font);
  text_layer_set_text_alignment(s_climbed_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_date_text_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_climbed_layer, GColorClear);
  text_layer_set_background_color(s_text_layer, GColorClear);
  text_layer_set_background_color(s_date_text_layer, GColorClear);
  s_line_layer = layer_create(bounds);
  layer_set_update_proc(s_line_layer, graphics_update_proc);
  layer_add_child(window_layer, text_layer_get_layer(s_climbed_layer));
  layer_add_child(window_layer, s_line_layer);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_text_layer));

}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  text_layer_destroy(s_date_text_layer);
  text_layer_destroy(s_climbed_layer);
  layer_destroy(s_line_layer);
  
}

static void update_time() {
    time_t temp = time(NULL);
    struct tm* tick_time = localtime(&temp);
    strftime(s_date_buffer, sizeof(s_date_buffer), "%a, %b %d", tick_time);
    strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
        "%H:%M" : "%I:%M", tick_time);
      int display_val = (int)(percent_climbed * 100);



    GRect bounds = layer_get_bounds(window_get_root_layer(s_window));
    if(climbingUp)
    {
      layer_set_frame(text_layer_get_layer(s_climbed_layer), GRect(10, 60, 70, 30));
    }
    else
    {
      layer_set_frame(text_layer_get_layer(s_climbed_layer), GRect(bounds.size.w - 10, 60, 70, 30));
    }
    snprintf(s_climbed_buffer, sizeof(s_climbed_buffer), "%% %d", display_val);
    text_layer_set_text(s_climbed_layer, s_climbed_buffer);
    text_layer_set_text(s_text_layer, s_buffer);
    text_layer_set_text(s_date_text_layer, s_date_buffer);
}
static void tick_handler(struct tm* tick_time, TimeUnits units_changed) {
    update_time();
}


static void prv_init(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });


  const bool animated = true;
  window_stack_push(s_window, animated);
  update_time();
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void prv_deinit(void) {
  window_destroy(s_window);
  //fonts_unload_custom_font(s_generic_font);
  //fonts_unload_custom_font(s_generic_small_font);
}

int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}