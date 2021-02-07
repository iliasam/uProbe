
/* Includes ------------------------------------------------------------------*/
#include "config.h"
#include "mode_controlling.h"
#include "adc_controlling.h"
#include "data_processing.h"
#include "display_functions.h"
#include "stdio.h"
#include "stdlib.h"

#include "slow_scope.h"

// Number of sampled points to skip
#define SLOW_SCOPE_START_OFFSET         (8)

//Number of displayed points
#define SLOW_SCOPE_POINT_CNT            (DISP_WIDTH)

//Header is part with capture text
#define SLOW_SCOPE_HEADER_HEIGHT        (9)

//For 1 sec period
#define SLOW_SCOPE_X_GRID_PERIOD        (10)

//part of the display is closed by device case
#define SLOW_SCOPE_Y_END                (DISPLAY_HEIGHT - 3)

#define SLOW_SCOPE_ACTIVE_HEIGHT        (SLOW_SCOPE_Y_END - SLOW_SCOPE_HEADER_HEIGHT)



/* Private variables ---------------------------------------------------------*/
extern menu_mode_t main_menu_mode;
extern volatile cap_status_type adc_capture_status;
extern volatile uint16_t adc_raw_buffer0[ADC_BUFFER_SIZE];
extern volatile uint32_t ms_tick;

slow_scope_data_processing_state_t slow_scope_state = ADC_SLOW_IDLE;

adc_processed_data_t slow_scope_last_result;

//Circullar buffer
adc_processed_data_t slow_scope_points[SLOW_SCOPE_POINT_CNT];
uint16_t slow_scope_buf_pointer = 0;

//Maximum voltage at the scope
float slow_scope_max_voltage = 4.0;
//Voltage grid period
float slow_scope_grid_v = 1.0;

//Array used for setting voltage grid 
grid_mode_item_t slow_scope_grid_mode_items[] =
{
  // max voltage, grid step in V
  {3.0f, 1.0f},
  {4.0f, 1.0f},
  {6.0f, 2.0f},
  {10.0f, 2.0f},
  {15.0f, 5.0f},
  {30.0f, 10.0f},
};

//Capture enabled flag
uint8_t slow_scope_capture_en_flag = 1;

#define SLOW_SCOPE_GRID_ITEMS_CNT  (sizeof(slow_scope_grid_mode_items) / sizeof(grid_mode_item_t))

/* Private function prototypes -----------------------------------------------*/
void slow_scope_process_data(void);
void slow_scope_clear_active_zone(void);
void slow_scope_draw_grid(void);
void slow_scope_draw_voltage_grid(uint16_t x);
uint16_t slow_scope_get_y_from_voltage(float voltage);
void slow_scope_draw_signal(void);
uint16_t slow_scope_draw_edges(uint16_t x, adc_processed_data_t point);
void slow_scope_calcutate_grid_step(void);

/* Private functions ---------------------------------------------------------*/

// This function must be called when "main_menu_mode" is changed
// Switch capture mode
void slow_scope_processing_main_mode_changed(void)
{
  slow_scope_state = ADC_SLOW_IDLE;
  if (main_menu_mode == MENU_MODE_SLOW_SCOPE)
  {
    adc_set_sample_rate(DATA_PROC_LOW_SAMPLE_RATE);
  }
}

void slow_scope_upper_button_pressed(void)
{
  // Toggle enabled flag
  slow_scope_capture_en_flag^= 1;
}


//Called from "data_processing_handler" in "data_processing.c"
void slow_scope_processing_handler(void)
{
  if (slow_scope_state == ADC_SLOW_IDLE)
  {
    adc_capture_start();
    slow_scope_state = ADC_SLOW_CAPTURE_RUNNING;
  }
  else if (slow_scope_state == ADC_SLOW_CAPTURE_RUNNING)
  {
    if (adc_capture_status == CAPTURE_DONE)
    {
      slow_scope_state = ADC_SLOW_PROCESSING_DATA;
      data_processing_correct_raw_data(data_processing_get_adc_offset());
      slow_scope_process_data();
      slow_scope_state = ADC_SLOW_PROCESSING_DATA_DONE;
    }
  }
}

void slow_scope_process_data(void)
{
  if (slow_scope_capture_en_flag == 0)
    return;
  
  //Analyse captured data and get single point result
  slow_scope_last_result = data_processing_extended(
    (uint16_t*)&adc_raw_buffer0[SLOW_SCOPE_START_OFFSET * 2], 
    (ADC_BUFFER_SIZE / 2 - SLOW_SCOPE_START_OFFSET * 2));
  
  //Add data to FIFO
  slow_scope_buf_pointer++;
  if (slow_scope_buf_pointer >= SLOW_SCOPE_POINT_CNT)
  {
    slow_scope_buf_pointer = 0;
  }
  slow_scope_points[slow_scope_buf_pointer] = slow_scope_last_result;
}

void slow_scope_draw_menu(menu_draw_type_t draw_type)
{
  static uint8_t new_data_pending = 0;
  
  if (draw_type == MENU_MODE_FULL_REDRAW)
  {
    display_clear_framebuffer();
    display_draw_string("SLOW SCOPE", 0, 0, FONT_SIZE_8, 0, COLOR_YELLOW);
    display_update();
  }
  else
  {
    if (slow_scope_state != ADC_SLOW_IDLE)
    {
      if (slow_scope_state == ADC_SLOW_PROCESSING_DATA_DONE)
      {
        slow_scope_state = ADC_SLOW_IDLE;
        new_data_pending = 1;
        return;
      }
    }
    
    if (new_data_pending == 1)
    {
      if ((slow_scope_capture_en_flag == 0) && ((ms_tick % 1000) < 500))
        display_draw_string("  STOPPED   ", 0, 0, FONT_SIZE_8, 0, COLOR_RED);
      else
        display_draw_string(" SLOW SCOPE", 0, 0, FONT_SIZE_8, 0, COLOR_YELLOW);
      
      char tmp_str[32];
      memset(tmp_str, 0, sizeof(tmp_str));
      
      sprintf(tmp_str, "%dV / %dV",(int)slow_scope_grid_v, (int)slow_scope_max_voltage);
      menu_shift_string_right(tmp_str, 9);
      display_draw_string(tmp_str, 90, 0, FONT_SIZE_8, 0, COLOR_WHITE);
      
      slow_scope_clear_active_zone();
      slow_scope_calcutate_grid_step();
      slow_scope_draw_grid();
      slow_scope_draw_signal();
      
      display_update();
      new_data_pending = 0;
    }
  }//PARTIAL_REDRAW
}

//Draw captured data from "slow_scope_points" FIFO
void slow_scope_draw_signal(void)
{
  uint16_t prev_y = SLOW_SCOPE_Y_END;//in pixels
  
  //read pointer of FIFO
  int16_t tmp_pointer = slow_scope_buf_pointer + 1;
  if (tmp_pointer >= SLOW_SCOPE_POINT_CNT)
      tmp_pointer = 0;
  
  prev_y = slow_scope_get_y_from_voltage(slow_scope_points[tmp_pointer].max_voltage);
  
  for (int16_t x = 0; x < SLOW_SCOPE_POINT_CNT; x++)
  {
    if (slow_scope_points[tmp_pointer].signal_type == ADC_SIGNAL_TYPE_STABLE)
    {
      float tmp_voltage = (slow_scope_points[tmp_pointer].max_voltage + slow_scope_points[tmp_pointer].min_voltage) / 2.0f;//average
      uint16_t point_y = slow_scope_get_y_from_voltage(tmp_voltage);
      display_set_pixel_color(x, point_y, COLOR_WHITE);
      
      //Draw line connecting current point and previous point
      if (abs(prev_y - point_y) > 2)
      {
        display_draw_vertical_line(x, prev_y, point_y, COLOR_WHITE);
      }
      prev_y = point_y;
    }
    else if (slow_scope_points[tmp_pointer].signal_type == ADC_SIGNAL_TYPE_SINGLE)
    {
      uint16_t max_y = slow_scope_get_y_from_voltage(
        slow_scope_points[tmp_pointer].max_voltage); //y is smaller
      uint16_t min_y = slow_scope_get_y_from_voltage(
        slow_scope_points[tmp_pointer].min_voltage);//y is bigger
      display_draw_vertical_line(x, min_y, max_y, COLOR_WHITE);
      
      prev_y = slow_scope_get_y_from_voltage(
        slow_scope_points[tmp_pointer].end_voltage);
    }
    else
    {
      slow_scope_draw_edges(x, slow_scope_points[tmp_pointer]);
      prev_y = slow_scope_get_y_from_voltage(
        slow_scope_points[tmp_pointer].end_voltage);
    }

    tmp_pointer++;
    if (tmp_pointer >= SLOW_SCOPE_POINT_CNT)
      tmp_pointer = 0;
  }
}

// Draw dotted line at position X - mean several signal edges
// Return minimum y
uint16_t slow_scope_draw_edges(uint16_t x, adc_processed_data_t point)
{
  uint16_t max_y = slow_scope_get_y_from_voltage(point.max_voltage); //y is smaller
  uint16_t min_y = slow_scope_get_y_from_voltage(point.min_voltage);//y is bigger
  
  for (uint16_t y = max_y; y <= min_y; y+= 2)
  {
    display_set_pixel_color(x, y, COLOR_GREEN);
  }
  display_set_pixel_color(x, min_y, COLOR_GREEN);//last point must be always on
  return min_y;
}

//-----------------------------------------------------------------------------

//Calculate needed grid max voltage and step by analysing captured data
void slow_scope_calcutate_grid_step(void)
{
  float max_voltage = 0.0;
  uint8_t grid_item = 0; 
  //find max voltage in all points
  for (uint16_t i = 0; i < SLOW_SCOPE_POINT_CNT; i++)
  {
    if (slow_scope_points[i].max_voltage > max_voltage)
    {
      max_voltage = slow_scope_points[i].max_voltage;
    }
  }
  
  for (uint8_t i = 0; i < (SLOW_SCOPE_GRID_ITEMS_CNT - 1); i++)
  {
    if (max_voltage > slow_scope_grid_mode_items[i].max_voltage)
      grid_item = i + 1;
  }
  
  slow_scope_max_voltage = slow_scope_grid_mode_items[grid_item].max_voltage;
  slow_scope_grid_v =  slow_scope_grid_mode_items[grid_item].grid_step;
}

void slow_scope_draw_grid(void)
{
  //Graw time grid - X
  int16_t x;
  for (x = (SLOW_SCOPE_POINT_CNT - 1); x >= 0; x = x - SLOW_SCOPE_X_GRID_PERIOD)
  {
    slow_scope_draw_voltage_grid(x);
  }
  
  display_draw_line(SLOW_SCOPE_Y_END, COLOR_BLUE);
}

// Draw voltage grid - dots in Y line (vertical)
void slow_scope_draw_voltage_grid(uint16_t x)
{
  float voltage_val = 0.0f;
  while (voltage_val < slow_scope_max_voltage)
  {
    uint16_t y_pos = slow_scope_get_y_from_voltage(voltage_val);
    display_set_pixel_color(x, y_pos, COLOR_YELLOW);
    voltage_val+= slow_scope_grid_v;
  }
}

//-----------------------------------------------------------------------------

//Convert voltage to y position (counted from upper line)
uint16_t slow_scope_get_y_from_voltage(float voltage)
{
  uint16_t pix_cnt = 
    (uint16_t)(voltage * (float)SLOW_SCOPE_ACTIVE_HEIGHT / slow_scope_max_voltage);
  
  if (pix_cnt > SLOW_SCOPE_ACTIVE_HEIGHT)
    pix_cnt = SLOW_SCOPE_ACTIVE_HEIGHT;
  
  return (SLOW_SCOPE_Y_END - pix_cnt);
}

void slow_scope_clear_active_zone(void)
{
  uint8_t y;
  for (y = SLOW_SCOPE_HEADER_HEIGHT; y < DISPLAY_HEIGHT; y++)
    display_draw_line(y, COLOR_BLACK);
}
