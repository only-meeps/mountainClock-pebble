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
static bool climbingUp = true;
static double percent_climbed = 0.0;
static double steps_per_side = 1000;
static int mountainsCompleted = 0;
static int mountainsCompletedToday = 0;
static uint32_t key_mountains_completed = 0;
static uint32_t key_mountains_completed_today = 1;
static uint32_t key_last_date_written = 2;
static int dummy_step_provider = 600;
static GPath **allPaths;
static GColor *allColors;
static int totalPathCount;
static GPoint start_point1;
static GPoint start_point2;
static GPoint end_point1;
static GPoint end_point2;
static GPoint start_middle;
static GPoint end_middle;

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

static void LogGPoint(GPoint point, int identifier) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Logging GPoint %d: (X = %d, Y = %d)",
          identifier, point.x, point.y);
}

static int Random(int Min, int Max) { return (rand() % (Max - Min + 1)) + Min; }

/*
static void init_all_lines(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int backgroundMountains = 17;
  int cloudsPerMountain = Random(1, 4);
  int mountainPoints = 2;
  int pointerPoints = 1;
  int cloudCount = 30;
  int totalPathCount = (backgroundMountains * 2) + cloudsPerMountain +
                       mountainPoints + cloudCount;
  int pathIdx = 0;
  allPaths = malloc(sizeof(GPath *) * totalPathCount);
  memset(allPaths, 0, sizeof(GPath *) * totalPathCount);
  allColors = malloc(sizeof(GColor) * totalPathCount);

  memset(allColors, 0, sizeof(GColor) * totalPathCount);
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
      GPoint(end_point2.x, end_point2.y + rightOffset.y);
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
      for (int j = 0; j < cloudsPerMountain; j++) {
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
        allPaths[pathIdx] = s_cloud_path;
        allColors[pathIdx] = GColorWhite;
        pathIdx++;
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

    allPaths[pathIdx] = s_light_path;
    allColors[pathIdx] = GColorBrightGreen;
    pathIdx++;

    allPaths[pathIdx] = s_dark_path;
    allColors[pathIdx] = GColorIslamicGreen;
    pathIdx++;
  }*/
static void redraw_lines(Layer *layer, GContext *ctx) {
  if (allPaths == NULL)
    return;
  GRect bounds = layer_get_bounds(layer);
  GPoint center = GPoint(0, 0);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Percent Climbed %d",
          (int)(percent_climbed * 100));
  LogGPoint(start_point1, 1);
  LogGPoint(start_point2, 2);

  if (climbingUp) {
    center = GPoint(
        start_point1.x + percent_climbed * (end_point1.x - start_point1.x),
        start_point1.y + percent_climbed * (end_point1.y - start_point1.y));
  } else {
    center = GPoint(
        start_point2.x + percent_climbed * (end_point2.x - start_point2.x),
        start_point2.y + percent_climbed * (end_point2.y - start_point2.y));
  }
  start_point1 = GPoint(-((int)steps_per_side / 2), 0);
  end_point1 = GPoint(bounds.size.w / 2, -((int)steps_per_side));

  start_point2 = GPoint(((int)steps_per_side / 2), 0);
  end_point2 = GPoint(bounds.size.w / 2, -((int)steps_per_side));

  start_middle = GPoint(bounds.size.w / 2, 0);

  GPoint *DarkPoints = malloc(sizeof(GPoint) * 3);
  DarkPoints[0] = start_point1;
  DarkPoints[1] = end_point1;
  DarkPoints[2] = start_middle;
  GPathInfo DarkInfo = {.num_points = 3, .points = DarkPoints};

  GPoint *LightPoints = malloc(sizeof(GPoint) * 3);
  LightPoints[0] = start_point2;
  LightPoints[1] = end_point2;
  LightPoints[2] = start_middle;
  GPathInfo LightInfo = {.num_points = 3, .points = LightPoints};
  double clamped_y = (bounds.size.h - center.y) - 60;
  if (clamped_y < 0) {
    clamped_y = 0;
  }
  LogGPoint(center, 3);
  center = GPoint(center.x, clamped_y);
  LogGPoint(center, 4);
  GPoint pointing_spot = GPoint(0, 0);
  GPoint Pointers[2];
  if (climbingUp) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Climbing up");
    pointing_spot = GPoint(
        ((start_point1.x + (end_point1.x - start_point1.x) * percent_climbed) +
         center.x),
        (start_point1.y + (end_point1.y - start_point1.y) * percent_climbed) +
            center.y);
    Pointers[0] = GPoint(10, 60);
    Pointers[1] = GPoint(80, 60);
    // Pointers[2] = pointing_spot;
  } else {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Climbing down");
    pointing_spot = GPoint(
        ((start_point2.x + (end_point2.x - start_point2.x) * percent_climbed) +
         center.x),
        (start_point2.y + (end_point1.y - start_point2.y) * percent_climbed) +
            center.y);
    Pointers[0] = GPoint(bounds.size.w - 10, 60);
    Pointers[1] = GPoint(bounds.size.w - 80, 60);
    // Pointers[2] = pointing_spot;
  }

  for (int i = 0; i < totalPathCount; i++) {
    if (allPaths[i] == NULL)
      continue;
    graphics_context_set_fill_color(ctx, allColors[i]);
    gpath_move_to(allPaths[i], GPoint(0, 0));
    gpath_move_to(allPaths[i], center);
    gpath_draw_filled(ctx, allPaths[i]);
  }
  GPathInfo pointerLineInfo = {.num_points = 2, .points = Pointers};
  GPath *pointerLinePath = gpath_create(&pointerLineInfo);
  gpath_draw_outline_open(ctx, pointerLinePath);
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
  GPath *s_diamond_path = gpath_create(&diamondInfo);
  graphics_context_set_fill_color(
      ctx, PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorBlack));
  gpath_draw_filled(ctx, s_diamond_path);
  gpath_destroy(s_diamond_path);
  gpath_destroy(pointerLinePath);
}
static void init_all_lines(Layer *layer, GContext *ctx) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Init");
  if (allPaths != NULL) {
    free(allPaths);
  }
  if (allColors != NULL) {
    free(allColors);
  }
  totalPathCount = 0;
  GRect bounds = layer_get_bounds(layer);
  int backgroundMountains = 17;
  int mountainPoints = 2;
  int pointerPoints = 1;
  int cloudCount = 30;
  totalPathCount = (backgroundMountains * 2) + mountainPoints + cloudCount;
  int pathIdx = 0;
  allPaths = malloc(sizeof(GPath *) * totalPathCount);
  memset(allPaths, 0, sizeof(GPath *) * totalPathCount);

  allColors = malloc(sizeof(GColor) * totalPathCount);
  memset(allColors, 0, sizeof(GColor) * totalPathCount);

  start_point1 = GPoint(-steps_per_side / 2, 0);
  end_point1 = GPoint(bounds.size.w / 2, -steps_per_side - bounds.size.h);

  start_point2 = GPoint(steps_per_side / 2, 0);
  end_point2 = GPoint(bounds.size.w / 2, -steps_per_side - bounds.size.h);

  start_middle = GPoint(bounds.size.w / 2, 0);
  end_middle = GPoint(bounds.size.w / 2, -steps_per_side);

  GPoint cloudBoundsBottomLeft = GPoint(start_point1.x, 0);
  GPoint cloudBoundsTopRight = GPoint(start_point2.x, start_middle.y);
  // LogGPoint(start_point1, 1);
  // LogGPoint(start_point2, 2);
  GPoint *DarkPoints = malloc(sizeof(GPoint) * 3);
  DarkPoints[0] = start_point1;
  DarkPoints[1] = end_point1;
  DarkPoints[2] = start_middle;
  GPathInfo DarkInfo = {.num_points = 3, .points = DarkPoints};

  GPoint *LightPoints = malloc(sizeof(GPoint) * 3);
  LightPoints[0] = start_point2;
  LightPoints[1] = end_point2;
  LightPoints[2] = start_middle;
  GPathInfo LightInfo = {.num_points = 3, .points = LightPoints};

  for (int i = 0; i < backgroundMountains; i++) {
    GPoint mountainBottomMiddle = GPoint(
        Random(cloudBoundsBottomLeft.x, cloudBoundsTopRight.x), bounds.size.h);

    GPoint mountainTopMiddle =
        GPoint(mountainBottomMiddle.x, Random(-50, -100));

    float size = Random(40, 80);

    GPoint mountainBottomLeft =
        GPoint(mountainBottomMiddle.x - size, mountainBottomMiddle.y);

    GPoint mountainBottomRight =
        GPoint(mountainBottomMiddle.x + size, mountainBottomMiddle.y);
    srand(i);

    GPoint *mountainDarkPoints = malloc(sizeof(GPoint) * 3);
    mountainDarkPoints[0] =
        GPoint(mountainBottomMiddle.x, mountainBottomMiddle.y);
    mountainDarkPoints[1] = GPoint(mountainTopMiddle.x, mountainTopMiddle.y);

    mountainDarkPoints[2] = GPoint(mountainBottomLeft.x, mountainBottomLeft.y);

    GPoint *mountainLightPoints = malloc(sizeof(GPoint) * 3);
    mountainLightPoints[0] =
        GPoint(mountainBottomMiddle.x - 1, mountainBottomMiddle.y);

    mountainLightPoints[1] =
        GPoint(mountainTopMiddle.x - 1, mountainTopMiddle.y);

    mountainLightPoints[2] =
        GPoint(mountainBottomRight.x, mountainBottomRight.y);

    GPathInfo lightInfo = {.num_points = 3, .points = mountainLightPoints};
    GPathInfo darkInfo = {.num_points = 3, .points = mountainDarkPoints};

    GPath *s_light_path = gpath_create(&lightInfo);
    GPath *s_dark_path = gpath_create(&darkInfo);

    allPaths[pathIdx] = s_light_path;
    allColors[pathIdx] = GColorBrightGreen;
    pathIdx++;

    allPaths[pathIdx] = s_dark_path;
    allColors[pathIdx] = GColorIslamicGreen;
    pathIdx++;
  }

  GPath *s_dark_shade_path = gpath_create(&DarkInfo);
  GPath *s_light_shade_path = gpath_create(&LightInfo);
  allPaths[pathIdx] = s_dark_shade_path;
  allColors[pathIdx] = PBL_IF_COLOR_ELSE(GColorKellyGreen, GColorBlack);
  pathIdx++;
  allPaths[pathIdx] = s_light_shade_path;
  allColors[pathIdx] = PBL_IF_COLOR_ELSE(GColorMintGreen, GColorWhite);
  pathIdx++;

  for (int i = 0; i < cloudCount; i++) {
    srand(i);
    GPoint *cloudPoints = malloc(sizeof(GPoint) * 6);
    int cloudSize = Random(30, 100);
    int cloudHeight = Random(20, 25);

    GPoint cloudPositionMiddle =
        GPoint(Random(cloudBoundsBottomLeft.x, cloudBoundsTopRight.x),
               Random(cloudBoundsBottomLeft.y, cloudBoundsTopRight.y));
    cloudPoints[0] = GPoint((cloudPositionMiddle.x) - (cloudSize / 2),
                            (cloudPositionMiddle.y) - (cloudHeight / 2));
    cloudPoints[1] = GPoint((cloudPositionMiddle.x) + (cloudSize / 2),
                            (cloudPositionMiddle.y) - (cloudHeight / 2));
    cloudPoints[2] = GPoint((cloudPositionMiddle.x) + (cloudSize / 2) + 10,
                            (cloudPositionMiddle.y));
    cloudPoints[3] = GPoint((cloudPositionMiddle.x) + (cloudSize / 2),
                            (cloudPositionMiddle.y) + (cloudHeight / 2));
    cloudPoints[4] = GPoint((cloudPositionMiddle.x) - (cloudSize / 2),
                            (cloudPositionMiddle.y) + (cloudHeight / 2));
    cloudPoints[5] = GPoint((cloudPositionMiddle.x) - (cloudSize / 2) - 10,
                            (cloudPositionMiddle.y));

    GPathInfo cloudInfo = {.num_points = 6, .points = cloudPoints};
    GPath *s_cloud_path = gpath_create(&cloudInfo);
    allPaths[pathIdx] = s_cloud_path;
    allColors[pathIdx] = GColorWhite;
    pathIdx++;
  }
}
static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Started");
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
  layer_set_update_proc(s_line_layer, redraw_lines);
  layer_add_child(window_layer, s_line_layer);
  layer_add_child(window_layer, text_layer_get_layer(s_steps_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_climbed_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_text_layer));

  init_all_lines(s_line_layer, NULL);
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

  time_t time_end = time(NULL);
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
    dummy_step_provider = 0;
    init_all_lines(s_line_layer, NULL);
  }
  layer_mark_dirty(s_line_layer);
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
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void prv_deinit(void) {
  window_destroy(s_window);
  if (allPaths == NULL)
    return;

  for (int i = 0; i < totalPathCount; i++) {
    if (allPaths[i] != NULL) {
      free((GPoint *)allPaths[i]->points);
      gpath_destroy(allPaths[i]);
    }
  }
  free(allPaths);
  allPaths = NULL;

  if (allColors) {
    free(allColors);
    allColors = NULL;
  }
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