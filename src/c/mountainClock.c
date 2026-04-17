#include "gcolor_definitions.h"
#include "text_animation_window.h"
#include <pebble.h>
#include <stdio.h>
#include <stdlib.h>

enum Weather {
  cloudy,
  snowy,
  rainy,
  lightning,
};
static Window *s_window;
static TextLayer *s_text_layer;
static TextLayer *s_date_text_layer;
static TextLayer *s_climbed_layer;
static TextLayer *s_steps_layer;
static char s_buffer[32];
static char s_date_buffer[16];
static char s_climbed_buffer[8];
static char s_step_count_buffer[16];
static GFont s_generic_font;
static GFont s_generic_small_font;
static Layer *s_line_layer;
static bool climbingUp = false;
static double percent_climbed = 0.0;
static double steps_per_side = 1000;
static int mountainsCompleted = 0;
static int mountainsCompletedToday = 0;
static int mountain_seed;
static uint32_t key_mountains_completed = 0;
static uint32_t key_mountains_completed_today = 1;
static uint32_t key_last_date_written = 2;
static uint32_t key_time_started = 3;
static uint32_t key_time_finished = 4;
static uint32_t key_mountain_seed = 5;
static int dummy_step_provider = 200;
static enum Weather currentWeather = rainy;

uint32_t pebble_isqrt(uint32_t n) {
  if (n < 2)
    return n;

  uint32_t x = n;
  uint32_t y = (x + 1) / 2;
  while (y < x) {
    x = y;
    y = (x + n / x) / 2;
  }
  return x;
}
void srand(unsigned int seed);

static int Random(int Min, int Max) { return (rand() % (Max - Min + 1)) + Min; }
static void graphics_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint start_point1 = GPoint(-steps_per_side / 2, bounds.size.h);
  GPoint end_point1 = GPoint(bounds.size.w / 2, -steps_per_side);

  GPoint start_point2 = GPoint(steps_per_side / 2, bounds.size.h);
  GPoint end_point2 = GPoint(bounds.size.w / 2, -steps_per_side);

  GPoint start_middle = GPoint(bounds.size.w / 2, bounds.size.h);
  GPoint end_middle = GPoint(bounds.size.w / 2, -steps_per_side);
  GPoint center = GPoint(0, 0);

  GPoint cloudBoundsBottomLeft = GPoint(start_point1.x, 0);
  GPoint cloudBoundsTopRight = GPoint(start_point2.x, start_middle.y);
  if (climbingUp) {
    center = GPoint(start_point1.x +
                        (end_point1.x - start_point1.x) * percent_climbed,
                    start_point1.y + 70 +
                        (end_point1.y - start_point1.y) * percent_climbed);
  } else {
    center = GPoint(start_point2.x +
                        (end_point2.x - start_point2.x) * percent_climbed,
                    start_point2.y + 70 +
                        (end_point2.y - start_point2.y) * percent_climbed);
  }

  double clamped_y = (bounds.size.h - center.y) - 30;
  if (clamped_y < 0) {
    clamped_y = 0;
  }
  GPoint leftOffset =
      GPoint((bounds.size.w - center.x) - bounds.size.w / 2, clamped_y);
  GPoint rightOffset =
      GPoint(((bounds.size.w - center.x) - bounds.size.w / 2), clamped_y);
  start_point1 =
      GPoint(start_point1.x + leftOffset.x, start_point1.y + leftOffset.y);
  end_point1 = GPoint(end_point1.x + leftOffset.x, end_point1.y + leftOffset.y);
  start_point2 =
      GPoint(start_point2.x + rightOffset.x, start_point2.y + rightOffset.y);
  end_point2 =
      GPoint(end_point2.x + rightOffset.x, end_point2.y + rightOffset.y);
  start_middle =
      GPoint(start_middle.x + rightOffset.x, start_middle.y + rightOffset.y);
  end_middle = GPoint(end_middle.x + leftOffset.x, end_middle.y + leftOffset.y);
  GPoint DarkPoints[] = {start_point1, end_middle, start_middle};
  GPathInfo DarkInfo = {.num_points = 3, .points = DarkPoints};
  GPoint LightPoints[] = {start_point2, end_point2, start_middle};
  GPathInfo LightInfo = {.num_points = 3, .points = LightPoints};
  GPoint pointing_spot = GPoint(0, 0);
  GPoint Pointers[2];
  if (climbingUp) {
    pointing_spot = GPoint(
        start_point1.x + (end_point1.x - start_point1.x) * percent_climbed,
        start_point1.y + (end_point1.y - start_point1.y) * percent_climbed);
    Pointers[0] = GPoint(10, 60);
    Pointers[1] = GPoint(80, 60);
    // Pointers[2] = pointing_spot;
  } else {
    pointing_spot = GPoint(
        start_point2.x + (end_point2.x - start_point2.x) * percent_climbed,
        start_point2.y + (end_point1.y - start_point2.y) * percent_climbed);
    Pointers[0] = GPoint(bounds.size.w - 10, 60);
    Pointers[1] = GPoint(bounds.size.w - 80, 60);
    // Pointers[2] = pointing_spot;
  }

  int backgroundMountains = 17;

  for (int i = 0; i < backgroundMountains; i++) {
    GPoint mountainBottomMiddle = GPoint(
        Random(cloudBoundsBottomLeft.x, cloudBoundsTopRight.x), bounds.size.h);
    GPoint mountainTopMiddle =
        GPoint(mountainBottomMiddle.x, Random(-50, -100));
    float size = Random(40, 80);
    GPoint mountainBottomLeft =
        GPoint(mountainBottomMiddle.x - size, bounds.size.h);
    GPoint mountainBottomRight =
        GPoint(mountainBottomMiddle.x + size, bounds.size.h);
    int cloudChance = Random(0, 1);
    if (cloudChance == 0) {
      int clouds = Random(1, 4);
      for (int j = 0; j < clouds; j++) {
        srand(i + j);
        GPoint cloudPoints[6];
        int cloudSize = Random(30, 100);
        int cloudHeight = Random(20, 25);

        GPoint cloudPositionMiddle =
            GPoint(Random(mountainBottomLeft.x, mountainBottomRight.x),
                   Random(mountainBottomMiddle.y, mountainTopMiddle.y));
        cloudPoints[0] =
            GPoint((cloudPositionMiddle.x + center.x) - (cloudSize / 2),
                   (cloudPositionMiddle.y + center.y) - (cloudHeight / 2));
        cloudPoints[1] =
            GPoint((cloudPositionMiddle.x + center.x) + (cloudSize / 2),
                   (cloudPositionMiddle.y + center.y) - (cloudHeight / 2));
        cloudPoints[2] =
            GPoint((cloudPositionMiddle.x + center.x) + (cloudSize / 2) + 10,
                   (cloudPositionMiddle.y + center.y));
        cloudPoints[3] =
            GPoint((cloudPositionMiddle.x + center.x) + (cloudSize / 2),
                   (cloudPositionMiddle.y + center.y) + (cloudHeight / 2));
        cloudPoints[4] =
            GPoint((cloudPositionMiddle.x + center.x) - (cloudSize / 2),
                   (cloudPositionMiddle.y + center.y) + (cloudHeight / 2));
        cloudPoints[5] =
            GPoint((cloudPositionMiddle.x + center.x) - (cloudSize / 2) - 10,
                   (cloudPositionMiddle.y + center.y));

        GPathInfo cloudInfo = {.num_points = 6, .points = cloudPoints};
        GPath *s_cloud_path = gpath_create(&cloudInfo);
        graphics_context_set_fill_color(ctx, GColorWhite);
        graphics_context_set_stroke_color(ctx, GColorBlack);
        graphics_context_set_stroke_width(ctx, 2);
        gpath_draw_filled(ctx, s_cloud_path);
        gpath_destroy(s_cloud_path);
      }
    }
    srand(i);

    GPoint mountainDarkPoints[3];
    mountainDarkPoints[0] = GPoint(mountainBottomMiddle.x + center.x,
                                   mountainBottomMiddle.y + center.y);
    mountainDarkPoints[1] =
        GPoint(mountainTopMiddle.x + center.x, mountainTopMiddle.y + center.y);
    mountainDarkPoints[2] = GPoint(mountainBottomLeft.x + center.x,
                                   mountainBottomLeft.y + center.y);

    GPoint mountainLightPoints[3];
    mountainLightPoints[0] = GPoint(mountainBottomMiddle.x + center.x - 1,
                                    mountainBottomMiddle.y + center.y);
    mountainLightPoints[1] = GPoint(mountainTopMiddle.x + center.x - 1,
                                    mountainTopMiddle.y + center.y);
    mountainLightPoints[2] = GPoint(mountainBottomRight.x + center.x,
                                    mountainBottomRight.y + center.y);

    GPathInfo lightInfo = {.num_points = 3, .points = mountainLightPoints};
    GPathInfo darkInfo = {.num_points = 3, .points = mountainDarkPoints};

    GPath *s_light_path = gpath_create(&lightInfo);
    GPath *s_dark_path = gpath_create(&darkInfo);

    graphics_context_set_fill_color(ctx, GColorBrightGreen);
    gpath_draw_filled(ctx, s_light_path);
    graphics_context_set_fill_color(ctx, GColorIslamicGreen);
    gpath_draw_filled(ctx, s_dark_path);
    gpath_destroy(s_light_path);
    gpath_destroy(s_dark_path);
  }

  float diamondSize = 10;

  GPoint diamondTop =
      GPoint(pointing_spot.x, pointing_spot.y - (diamondSize * 1.5));
  GPoint diamondBottom = pointing_spot;
  GPoint diamondLeft = GPoint(pointing_spot.x - (diamondSize * 0.5),
                              pointing_spot.y - (diamondSize));
  GPoint diamondRight = GPoint(pointing_spot.x + (diamondSize * 0.5),
                               pointing_spot.y - (diamondSize));
  ;
  GPoint diamond[] = {diamondTop, diamondLeft, diamondBottom, diamondRight};

  GPathInfo diamondInfo = {.num_points = 4, .points = diamond};
  GPathInfo PointerInfo = {.num_points = 2, .points = Pointers};
  GPath *s_diamond_path = gpath_create(&diamondInfo);
  GPath *s_dark_shade_path = gpath_create(&DarkInfo);
  GPath *s_light_shade_path = gpath_create(&LightInfo);
  GPath *s_pointer_path = gpath_create(&PointerInfo);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 2);

  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_context_set_fill_color(ctx,
                                  PBL_IF_COLOR_ELSE(GColorGreen, GColorBlack));
  gpath_draw_filled(ctx, s_dark_shade_path);
  // gpath_draw_outline(ctx, s_dark_shade_path);
  graphics_context_set_fill_color(
      ctx, PBL_IF_COLOR_ELSE(GColorMintGreen, GColorWhite));
  gpath_draw_filled(ctx, s_light_shade_path);
  gpath_draw_filled(ctx, s_light_shade_path);

  gpath_destroy(s_dark_shade_path);
  gpath_destroy(s_light_shade_path);

  int cloudCount = 30;
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Min %d %d", start_point2.x, start_point2.y);
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Max %d %d", start_point1.x, start_point1.y);
  for (int i = 0; i < cloudCount; i++) {
    srand(i);
    GPoint cloudPoints[6];
    int cloudSize = Random(30, 100);
    int cloudHeight = Random(20, 25);

    GPoint cloudPositionMiddle =
        GPoint(Random(cloudBoundsBottomLeft.x, cloudBoundsTopRight.x),
               Random(cloudBoundsBottomLeft.y, cloudBoundsTopRight.y));
    cloudPoints[0] =
        GPoint((cloudPositionMiddle.x + center.x) - (cloudSize / 2),
               (cloudPositionMiddle.y + center.y) - (cloudHeight / 2));
    cloudPoints[1] =
        GPoint((cloudPositionMiddle.x + center.x) + (cloudSize / 2),
               (cloudPositionMiddle.y + center.y) - (cloudHeight / 2));
    cloudPoints[2] =
        GPoint((cloudPositionMiddle.x + center.x) + (cloudSize / 2) + 10,
               (cloudPositionMiddle.y + center.y));
    cloudPoints[3] =
        GPoint((cloudPositionMiddle.x + center.x) + (cloudSize / 2),
               (cloudPositionMiddle.y + center.y) + (cloudHeight / 2));
    cloudPoints[4] =
        GPoint((cloudPositionMiddle.x + center.x) - (cloudSize / 2),
               (cloudPositionMiddle.y + center.y) + (cloudHeight / 2));
    cloudPoints[5] =
        GPoint((cloudPositionMiddle.x + center.x) - (cloudSize / 2) - 10,
               (cloudPositionMiddle.y + center.y));

    GPathInfo cloudInfo = {.num_points = 6, .points = cloudPoints};
    GPath *s_cloud_path = gpath_create(&cloudInfo);
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_context_set_stroke_width(ctx, 2);
    gpath_draw_filled(ctx, s_cloud_path);
    gpath_destroy(s_cloud_path);
  }
  graphics_context_set_fill_color(
      ctx, PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorBlack));
  gpath_draw_filled(ctx, s_diamond_path);
  gpath_destroy(s_diamond_path);
  gpath_draw_outline_open(ctx, s_pointer_path);
  gpath_destroy(s_pointer_path);
}
static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorElectricBlue);
  GRect bounds = layer_get_bounds(window_layer);
  s_generic_font = fonts_get_system_font(FONT_KEY_LECO_28_LIGHT_NUMBERS);
  s_text_layer = text_layer_create(GRect(0, bounds.size.h - 65, 80, 30));
  s_generic_small_font = fonts_get_system_font(FONT_KEY_GOTHIC_24);
  s_climbed_layer = text_layer_create(GRect(10, 60, 70, 30));
  s_steps_layer = text_layer_create(GRect(10, 40, 70, 30));
  s_date_text_layer = text_layer_create(GRect(0, bounds.size.h - 30, 80, 30));

  text_layer_set_font(s_climbed_layer, s_generic_small_font);
  text_layer_set_font(s_text_layer, s_generic_font);
  text_layer_set_font(s_date_text_layer, s_generic_small_font);
  text_layer_set_font(s_steps_layer, s_generic_small_font);

  text_layer_set_text_alignment(s_climbed_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_date_text_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_steps_layer, GTextAlignmentCenter);

  text_layer_set_background_color(s_climbed_layer, GColorClear);
  text_layer_set_background_color(s_text_layer, GColorClear);
  text_layer_set_background_color(s_date_text_layer, GColorClear);
  text_layer_set_background_color(s_steps_layer, GColorClear);

  s_line_layer = layer_create(bounds);
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  int lastDaySet = tick_time->tm_yday;
  persist_write_int(key_last_date_written, lastDaySet);
  if (persist_exists(key_mountains_completed)) {
    mountainsCompleted = persist_read_int(key_mountains_completed);
    mountainsCompletedToday = persist_read_int(key_mountains_completed_today);
  } else {
    persist_write_int(key_mountains_completed, 0);
    persist_write_int(key_mountains_completed_today, 0);
  }
  layer_set_update_proc(s_line_layer, graphics_update_proc);
  layer_add_child(window_layer, s_line_layer);
  layer_add_child(window_layer, text_layer_get_layer(s_steps_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_climbed_layer));
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
  struct tm *tick_time = localtime(&temp);
  strftime(s_date_buffer, sizeof(s_date_buffer), "%a, %b %d", tick_time);
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M",
           tick_time);

  time_t time_start = time_start_of_today();
  time_t time_end = time(NULL);
  struct tm *utc_tm = gmtime(&time_end);
  // HealthValue value = health_service_sum_averaged(
  //     HealthMetricStepCount, time_start, time_end,
  //     HealthServiceTimeScopeDaily);
  int value = dummy_step_provider;
  int display_val = 0;

  if ((int)value >= steps_per_side && climbingUp == true) {
    climbingUp = false;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "summited mountain!");
    char text1[32] = "You summited a mountain";
    char bottom_text[32];
    snprintf(bottom_text, sizeof(bottom_text), " you summited in %d steps",
             value);
    text_animation_window_push(text1, bottom_text);
  } else if ((int)value >= steps_per_side * 2 && climbingUp == false) {
    vibes_long_pulse();
    mountainsCompleted += 1;
    persist_write_int(key_mountains_completed, mountainsCompleted);
    steps_per_side += 200;
  }
  percent_climbed = (value / steps_per_side);
  if (climbingUp) {
    display_val = (int)(percent_climbed * 100);
  } else {
    display_val = ((int)(percent_climbed * 100));
  }

  GRect bounds = layer_get_bounds(window_get_root_layer(s_window));
  if (climbingUp) {
    layer_set_frame(text_layer_get_layer(s_climbed_layer),
                    GRect(10, 60, 70, 30));
    layer_set_frame(text_layer_get_layer(s_steps_layer), GRect(5, 30, 80, 30));
  } else {
    layer_set_frame(text_layer_get_layer(s_climbed_layer),
                    GRect(bounds.size.w - 80, 60, 70, 30));
    layer_set_frame(text_layer_get_layer(s_steps_layer),
                    GRect(bounds.size.w - 85, 30, 80, 30));
  }
  snprintf(s_climbed_buffer, sizeof(s_climbed_buffer), "%% %d", display_val);
  snprintf(s_step_count_buffer, sizeof(s_step_count_buffer), "%d steps",
           (int)value);
  text_layer_set_text(s_steps_layer, s_step_count_buffer);
  text_layer_set_text(s_climbed_layer, s_climbed_buffer);
  text_layer_set_text(s_text_layer, s_buffer);
  text_layer_set_text(s_date_text_layer, s_date_buffer);
  dummy_step_provider += 10;
}
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void prv_init(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
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
  // fonts_unload_custom_font(s_generic_font);
  // fonts_unload_custom_font(s_generic_small_font);
}

int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p",
          s_window);

  app_event_loop();
  prv_deinit();
}