
/* Includes ------------------------------------------------------------------*/
#include "config.h"
#include "mode_controlling.h"
#include "adc_controlling.h"
#include "data_processing.h"
#include "display_functions.h"
#include "comparator_handling.h"
#include "stdio.h"
#include "stdlib.h"
#include "stm32f30x_it.h"

#include "baud_meter.h"

/* Private typedef -----------------------------------------------------------*/

// Voltage in V
#define BAUD_METER_THRESHOLD_VOLTAGE    (1.5f)
#define BAUD_METER_ROUNDING_ACCURACY    (0.1f) //10%


/* Private variables ---------------------------------------------------------*/
extern menu_mode_t main_menu_mode;

extern volatile uint32_t comparator_irq_dwt_buff[COMP_INTERRUPTS_DWT_BUF_SIZE];
extern volatile uint8_t comparator_irq_dwt_buff_full;
extern volatile uint16_t comparator_irq_counter;//Interrupts counter

// Raw baudrate value, bod
float baud_meter_current_raw_baud = 0.0;
// Rounded to a closer from table value, bod
uint32_t baud_meter_current_rounded_baud = 0;

uint32_t baud_meter_update_timestamp = 0;

// State of baud measurement
baud_meter_processing_state_t baud_meter_processing_state = BAUD_PROCESSING_IDLE;
uint32_t baud_meter_timeout_timer = 0;
uint8_t baud_meter_fast_data_detected_flag = 0;

// Table with common baudrates
const uint32_t baud_meter_bauds_table[] = 
  {1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200, 230400, 0};

/* Private function prototypes -----------------------------------------------*/
void baud_meter_update_period(uint32_t new_bit_length);
void baud_meter_round_baudrate(void);
uint32_t baud_meter_get_min_length(void);

/* Private functions ---------------------------------------------------------*/

// This function must be called when "main_menu_mode" is changed
// Switch capture mode
void baud_meter_processing_main_mode_changed(void)
{
  if (main_menu_mode == MENU_MODE_BAUD_METER)
  {
    comparator_init(USE_INTERUPT_MODE);
    comparator_set_threshold(BAUD_METER_THRESHOLD_VOLTAGE);
  }
}

//Called from "data_processing_handler" in "data_processing.c"
void baud_meter_processing_handler(void)
{
  if (baud_meter_processing_state == BAUD_PROCESSING_IDLE)
  {
    baud_meter_timeout_timer = ms_tick;
    baud_meter_processing_state = BAUD_PROCESSING_CAPTURE_FAST_RUNNING; //wait for comparator interrupt
    comparator_start_wait_interrupt();
  }
  else if (baud_meter_processing_state == BAUD_PROCESSING_CAPTURE_FAST_RUNNING)
  {
    if (comparator_irq_dwt_buff_full)
    {
      uint32_t min_length = baud_meter_get_min_length();
      baud_meter_update_period(min_length);
      baud_meter_processing_state = BAUD_PROCESSING_IDLE;
    }
    else
    {
      uint32_t time_diff = ms_tick - baud_meter_timeout_timer;
      if ((time_diff > 1500) && (comparator_irq_counter < 10))
      {
        baud_meter_update_period(0);//fail
        baud_meter_processing_state = BAUD_PROCESSING_IDLE;
      }
      else if ((time_diff > 2000) && (comparator_irq_counter < 50))
      {
        baud_meter_update_period(0);//fail
        baud_meter_processing_state = BAUD_PROCESSING_IDLE;
      }
      else if (time_diff > 4000)
      {
        baud_meter_update_period(0);//fail
        baud_meter_processing_state = BAUD_PROCESSING_IDLE;
      }
    }
  }
}

uint32_t baud_meter_get_min_length(void)
{
  uint32_t min_length_value = 0xFFFFFFFF;
  for (uint16_t i = 0; i < (comparator_irq_counter - 1); i++)
  {
    uint32_t curr_length = comparator_irq_dwt_buff[i + 1] - comparator_irq_dwt_buff[i];
    if (curr_length < min_length_value)
      min_length_value = curr_length;
  }
  
  return min_length_value;
}


// Try to find closest baudrate in the table
void baud_meter_round_baudrate(void)
{
  if (baud_meter_current_raw_baud == 0)
  {
    baud_meter_current_rounded_baud = 0;
    return;
  }
  
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

//Calculate "baud_meter_current_rounded_baud" value
void baud_meter_update_period(uint32_t new_bit_length)
{
  if (new_bit_length == 0)
  {
    baud_meter_current_raw_baud = 0;
    baud_meter_round_baudrate();
    return;
  }
  
  //*2 because only falling edges are used
  baud_meter_current_raw_baud = 
        (float)SystemCoreClock * 2 / new_bit_length;
  baud_meter_round_baudrate();
}
