
/* Includes ------------------------------------------------------------------*/
#include "config.h"
#include "mode_controlling.h"
#include "adc_controlling.h"
#include "data_processing.h"
#include "display_functions.h"
#include "stdio.h"
#include "stdlib.h"
#include "stm32f30x_it.h"

#include "baud_meter.h"

/* Private typedef -----------------------------------------------------------*/
// Number of sampled points to skip
#define BAUD_METER_START_OFFSET         (4)

// Voltage in V
#define BAUD_METER_THRESHOLD_VOLTAGE    (1.5f)

// Length in ADC periods (in slow mode). 
// If "min low length" less than this threshold than fast mode is needed
#define BAUD_METER_FAST_MODE_THRESHOLD  (15)

//Number of fast measurements to get final result
#define BAUD_METER_FAST_MEAS_TOTAL_CNT  (50)

#define BAUD_METER_ROUNDING_ACCURACY    (0.1f) //10%


/* Private variables ---------------------------------------------------------*/
extern menu_mode_t main_menu_mode;
extern uint32_t current_sample_rate;
extern volatile cap_status_type adc_capture_status;
extern volatile uint16_t adc_raw_buffer0[ADC_BUFFER_SIZE];

uint8_t baud_meter_inversion_enabled = 0;
uint32_t baud_meter_current_bit_length = 0;

float baud_meter_current_raw_baud = 0.0;
uint32_t baud_meter_current_rounded_baud = 0;

uint32_t baud_meter_update_timestamp = 0;
// Table for storing results of analysing data in fast mode
uint16_t baud_meter_fast_bit_length_table[BAUD_METER_FAST_MEAS_TOTAL_CNT];

// Count of fast(2MHz) measurements
uint8_t baud_meter_fast_meas_count = 0;

// State of baud measurement
baud_meter_processing_state_t baud_meter_processing_state = BAUD_PROCESSING_IDLE;

// Table with common baudrates
const uint32_t baud_meter_bauds_table[] = {1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200, 0};

/* Private function prototypes -----------------------------------------------*/
void baud_meter_process_captured_data(void);
uint16_t baud_meter_get_min_length(void);
void baud_meter_update_period(uint32_t new_bit_length);
void baud_meter_process_fast_captured_data(void);
void baud_meter_round_baudrate(void);

/* Private functions ---------------------------------------------------------*/

// This function must be called when "main_menu_mode" is changed
// Switch capture mode
void baud_meter_processing_main_mode_changed(void)
{
  if (main_menu_mode == MENU_MODE_BAUD_METER)
  {
    adc_set_sample_rate(DATA_PROC_SAMPLE_RATE_200K);
    current_sample_rate = DATA_PROC_SAMPLE_RATE_200K;
  }
}

//Called from "data_processing_handler" in "data_processing.c"
void baud_meter_processing_handler(void)
{
  if (baud_meter_processing_state == BAUD_PROCESSING_IDLE)
  {
    //First try in slow mode
    adc_set_sample_rate(DATA_PROC_SAMPLE_RATE_200K);
    current_sample_rate = DATA_PROC_SAMPLE_RATE_200K;
    adc_capture_start();
    
    baud_meter_processing_state = BAUD_PROCESSING_CAPTURE_SLOW_RUNNING;
  }
  else if (baud_meter_processing_state == BAUD_PROCESSING_CAPTURE_SLOW_RUNNING)
  {
    if (adc_capture_status == CAPTURE_DONE)
    {
      baud_meter_processing_state = BAUD_PROCESSING_DATA;//test
      data_processing_correct_raw_data(data_processing_get_adc_offset());
      baud_meter_process_captured_data();  
    }
  }
  else if (baud_meter_processing_state == BAUD_PROCESSING_CAPTURE_FAST_RUNNING)
  {
    if (adc_capture_status == CAPTURE_DONE)
    {
      data_processing_correct_raw_data(data_processing_get_adc_offset());
      baud_meter_process_captured_data();
    }
  }
}

//Processing and controlling capture mode (after ADC capture is done)
void baud_meter_process_captured_data(void)
{
  uint16_t min_bit_length = baud_meter_get_min_length();
  
  if (current_sample_rate == DATA_PROC_SAMPLE_RATE_200K)
  {
    // PROCESS CATA CAPTURED IN 200K MODE
    if (min_bit_length == 0xFFFF)
    {
      baud_meter_update_period(min_bit_length);
      baud_meter_processing_state = BAUD_PROCESSING_IDLE;
      return;
    }
    
    if (min_bit_length < BAUD_METER_FAST_MODE_THRESHOLD)
    {
      // Too fast signal detected, we need to switch to fas mode and do several measurements
      adc_set_sample_rate(DATA_PROC_SAMPLE_RATE_2M);
      current_sample_rate = DATA_PROC_SAMPLE_RATE_2M;
      baud_meter_fast_meas_count = 0;
      baud_meter_processing_state = BAUD_PROCESSING_CAPTURE_FAST_RUNNING;
      adc_capture_start();
    }
    else
    {
      baud_meter_update_period(min_bit_length);
      baud_meter_processing_state = BAUD_PROCESSING_IDLE;
    }
  }
  else if (current_sample_rate == DATA_PROC_SAMPLE_RATE_2M)
  {
    baud_meter_fast_bit_length_table[baud_meter_fast_meas_count] = min_bit_length;
    baud_meter_fast_meas_count++;
    
    if (baud_meter_fast_meas_count > BAUD_METER_FAST_MEAS_TOTAL_CNT)
    {
      //time to get final result
      baud_meter_process_fast_captured_data();
      baud_meter_processing_state = BAUD_PROCESSING_IDLE;
    }
  }
}

//Process data captured in the seriesof fast captures
void baud_meter_process_fast_captured_data(void)
{
  uint8_t i;
  uint16_t min_period = 0xFFFF;
  for (i = 0; i < BAUD_METER_FAST_MEAS_TOTAL_CNT; i++)
  {
    if ((baud_meter_fast_bit_length_table[i] < min_period) && 
        (baud_meter_fast_bit_length_table[i] > 1))
      min_period = baud_meter_fast_bit_length_table[i];
  }
  baud_meter_update_period(min_period);  
}


// Update "baud_meter_current_bit_length" value
// Small and values are bad, so they are updated with delay
void baud_meter_update_period(uint32_t new_bit_length)
{
  if ((ms_tick - baud_meter_update_timestamp) > 1000)
  {
    //No restrictions
    baud_meter_update_timestamp = ms_tick;
    baud_meter_current_bit_length = new_bit_length;
    baud_meter_current_raw_baud = 
        (float)current_sample_rate / baud_meter_current_bit_length;
    baud_meter_round_baudrate();
  }
  else
  {
    if ((new_bit_length > 2) && (new_bit_length < 0xFFFF))
    {
      baud_meter_update_timestamp = ms_tick;
      baud_meter_current_bit_length = new_bit_length;
      baud_meter_current_raw_baud = 
        (float)current_sample_rate / baud_meter_current_bit_length;
      baud_meter_round_baudrate();
    }
  }
}

// Try to fine close baudrate in the table
void baud_meter_round_baudrate(void)
{
  uint8_t i = 0;
  uint32_t arruracy = 0; 
  baud_meter_current_rounded_baud = (uint32_t)baud_meter_current_raw_baud;
  
  while(baud_meter_bauds_table[i] != 0)
  {
    arruracy = (uint32_t)((float)baud_meter_bauds_table[i] * BAUD_METER_ROUNDING_ACCURACY);
    int32_t error = 
      (int32_t)baud_meter_bauds_table[i] - (int32_t)baud_meter_current_raw_baud;
    if (abs(error) < arruracy)
      baud_meter_current_rounded_baud = baud_meter_bauds_table[i];
    i++;
  }
}


//Function calculate min length of low level
uint16_t baud_meter_get_min_length(void)
{
  uint16_t raw_threshold = 
    data_processing_volt_to_points(BAUD_METER_THRESHOLD_VOLTAGE);
  
  uint16_t i;//sample number
  uint8_t prev_logic_state;
  uint16_t falling_start_pos = 0;
  uint16_t min_low_length = 0xFFFF;
  
  for (i = BAUD_METER_START_OFFSET; i < (ADC_BUFFER_SIZE/2); i++)
  {
    uint8_t cur_logic_state = 0;
    uint16_t value =  adc_raw_buffer0[i*2];
    if (value > raw_threshold)
      cur_logic_state = 1;
    
    if (baud_meter_inversion_enabled)
      cur_logic_state^= 1;
    
    if (i == BAUD_METER_START_OFFSET)
      prev_logic_state = cur_logic_state;//start initialization
    
    if (prev_logic_state != cur_logic_state)//edge detected
    {
      if (cur_logic_state == 0) //falling edge
      {
        falling_start_pos = i;
      }
      else //rising edge
      {
        if (falling_start_pos != 0)//protection - notice - "i" start value > 0
        {
          uint16_t low_level_lenght = i - falling_start_pos;//length in ADC periods
          if (low_level_lenght < min_low_length)
            min_low_length = low_level_lenght;
        }
      }
    }
    
    prev_logic_state = cur_logic_state;
  }//for

  return min_low_length;
}