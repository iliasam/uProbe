
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
#include "hardware.h"
#include "math.h"

#include "freq_measurement.h"

/* Private typedef -----------------------------------------------------------*/

// Number of sampled points to skip
#define FREQ_CALIB_START_OFFSET         (8)

// Number of voltage measurement cycles
#define FREQ_CALIB_CYCLES_CNT           (10)

// If the voltage is lower, standart trigger value is used
#define FREQ_CALIB_MIN_VOLTAGE         (0.2f)

#define FREQ_MEAS_MIN_1SEC_FREQ_HZ      (5)

typedef enum
{
  FREQ_MEASURE_WAIT = 0,
  FREQ_MEASURE_TIMER_FULL = 1,
  FREQ_MEASURE_1SEC_PERIOD = 2,
} freq_measure_result_t;

/* Private variables ---------------------------------------------------------*/
freq_measurement_state_t freq_measurement_state = FREQ_MEASUREMENT_IDLE;
float freq_comparator_threshold_v = FREQ_TRIGGER_DEFAULT_V;
uint8_t freq_meas_calibration_counter = 0;
//Calculated frequency, Hz
uint32_t freq_measurement_calc_frequency = 0;

freq_measure_result_t freq_meas_result = FREQ_MEASURE_WAIT;

uint32_t freq_measure_start_dwt_counter = 0;
uint32_t freq_measure_start_ms_counter = 0;
uint32_t freq_measure_result_dwt_value = 0;

extern menu_mode_t main_menu_mode;
extern freq_meter_calib_state_t freq_meter_calib_state;
extern volatile uint16_t adc_raw_buffer0[ADC_BUFFER_SIZE];

/* Private function prototypes -----------------------------------------------*/
void freq_meter_trigger_handling(void);
void freq_measurement_start_measure(void);

void FREQ_MEAS_TIM_IRQ_HANDLER(void);


/* Private functions ---------------------------------------------------------*/

void FREQ_MEAS_TIM_IRQ_HANDLER(void)
{
  if (TIM_GetITStatus(FREQ_MEAS_TIM_NAME, TIM_IT_Update) == SET)
  {
    //timer is stopped  - one pulse mode
    uint32_t dwt_value = hardware_dwt_get();
    TIM_ClearITPendingBit(FREQ_MEAS_TIM_NAME, TIM_IT_Update);
    TIM_ITConfig(FREQ_MEAS_TIM_NAME, TIM_IT_Update, DISABLE);
    freq_measure_result_dwt_value = dwt_value - freq_measure_start_dwt_counter;
    freq_meas_result = FREQ_MEASURE_TIMER_FULL;
  }
}


void freq_measurement_init_timers(void)
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  
  FREQ_MEAS_TIM_CLK_INIT_F(FREQ_MEAS_TIM_CLK, ENABLE);
  
  //Ext clock input
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Pin = FREQ_MEAS_TIM_ETR_PIN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(FREQ_MEAS_TIM_ETR_GPIO, &GPIO_InitStructure);
  GPIO_PinAFConfig(
    FREQ_MEAS_TIM_ETR_GPIO, FREQ_MEAS_TIM_ETR_AF_SRC, FREQ_MEAS_TIM_ETR_AFIO);

  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInit(FREQ_MEAS_TIM_NAME, &TIM_TimeBaseStructure);
  TIM_SelectOnePulseMode(FREQ_MEAS_TIM_NAME, TIM_OPMode_Single);
  TIM_ETRClockMode2Config(
    FREQ_MEAS_TIM_NAME, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0);
  TIM_SelectInputTrigger(FREQ_MEAS_TIM_NAME, TIM_TS_ETRF);
    
  NVIC_InitStructure.NVIC_IRQChannel = FREQ_MEAS_TIM_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void freq_measurement_start_measure(void)
{
  freq_meas_result = FREQ_MEASURE_WAIT;
  freq_measure_start_ms_counter = ms_tick;
  TIM_ClearFlag(FREQ_MEAS_TIM_NAME, TIM_FLAG_Update);
  TIM_ITConfig(FREQ_MEAS_TIM_NAME, TIM_IT_Update, ENABLE);
  TIM_SetCounter(FREQ_MEAS_TIM_NAME, 0);
  freq_measure_start_dwt_counter = hardware_dwt_get();
  FREQ_MEAS_TIM_NAME->CR1 |= TIM_CR1_CEN; //enable
}

//Start measuring frequency
void freq_measurement_start_freq_capture(void)
{
  freq_measurement_state = FREQ_MEASUREMENT_IDLE;
}

void freq_measurement_processing_handler(void)
{
  if (freq_measurement_state == FREQ_MEASUREMENT_IDLE)
  {
    comparator_set_threshold(freq_comparator_threshold_v);
    freq_measurement_start_measure();
    
    freq_measurement_state = FREQ_MEASUREMENT_CAPTURE_RUNNING;
  }
  else if (freq_measurement_state == FREQ_MEASUREMENT_CAPTURE_RUNNING)
  {
    if (freq_meas_result != 0)
    {
      freq_measurement_state = FREQ_MEASUREMENT_PROCESSING_DATA;
      uint32_t ticks_in_us = SystemCoreClock / 1000000; //MCU ticks in 1us
      
      if (freq_meas_result == FREQ_MEASURE_TIMER_FULL)
      {
        float timer_count_time_us = (float)freq_measure_result_dwt_value / ticks_in_us;
        float frequncy_hz = (float)0xFFFF / timer_count_time_us * 1e6;
        freq_measurement_calc_frequency = (uint32_t)frequncy_hz;
      }
      else
      {
        //FREQ_MEASURE_1SEC_PERIOD
        if (FREQ_MEAS_TIM_NAME->CNT > FREQ_MEAS_MIN_1SEC_FREQ_HZ)
        {
          float timer_count_time_us = (float)freq_measure_result_dwt_value / ticks_in_us;
          float frequncy_hz = (float)FREQ_MEAS_TIM_NAME->CNT / timer_count_time_us * 1e6;
          freq_measurement_calc_frequency = (uint32_t)roundf(frequncy_hz);
        }
        else
        {
          freq_measurement_calc_frequency = 0;
        }
      }
      freq_measurement_state = FREQ_MEASUREMENT_PROCESSING_DATA_DONE;
    }
    else
    {
      uint32_t time_diff_ms = ms_tick - freq_measure_start_ms_counter;
      if (time_diff_ms > 1000) //1s
      {
        FREQ_MEAS_TIM_NAME->CR1 &= ~TIM_CR1_CEN; //disable
        uint32_t dwt_value = hardware_dwt_get();
        freq_measure_result_dwt_value = dwt_value - freq_measure_start_dwt_counter;
        freq_meas_result = FREQ_MEASURE_1SEC_PERIOD;
      }
    }
  }
  
  freq_meter_trigger_handling();
}


// This function must be called when "main_menu_mode" is changed
// Switch capture mode
void freq_measurement_main_mode_changed(void)
{
  if (main_menu_mode == MENU_MODE_FREQUENCY_METER)
  {
    comparator_init(USE_NO_EVENTS_COMP);
    freq_comparator_threshold_v = FREQ_TRIGGER_DEFAULT_V;
  }
}

// Conrolling process of trigger level detection
void freq_meter_trigger_handling(void)
{
  static freq_meter_calib_state_t prev_menu_mode = FREQ_METER_CALIB_IDLE;
  static uint32_t calibration_timer = 0;
  
  // Detect state changing
  if (prev_menu_mode != freq_meter_calib_state)
  {
    if (freq_meter_calib_state == FREQ_METER_CALIB_WAIT_START)
    {
      //Begin to wait while notification is displayed
      START_TIMER(calibration_timer, FREQ_METER_START_WAIT_DELAY);
    }
  }
  else
  {
    if (freq_meter_calib_state == FREQ_METER_CALIB_WAIT_START)
    {
      if (TIMER_ELAPSED(calibration_timer))
      {
        freq_meter_calib_state = FREQ_METER_CALIB_CAPTURE;//start capture
        data_processing_start_new_capture();
        freq_meas_calibration_counter = 0;
        comparator_min_voltage = 500.0f;
        comparator_max_voltage = 0.0f;
        menu_draw_frequency_meter_menu(MENU_MODE_FULL_REDRAW);
      }
    }
    else if (freq_meter_calib_state == FREQ_METER_CALIB_CAPTURE)//running voltage capture
    {
      if (data_processing_state == PROCESSING_DATA_DONE)//voltage measured
      {
        freq_meas_calibration_counter++;
        adc_processed_data_t tmp_result = data_processing_extended(
          (uint16_t*)&adc_raw_buffer0[FREQ_CALIB_START_OFFSET * 2], 
          (ADC_BUFFER_SIZE / 2 - FREQ_CALIB_START_OFFSET * 2));
        
        if (tmp_result.min_voltage < comparator_min_voltage)
          comparator_min_voltage = tmp_result.min_voltage;
        if (tmp_result.max_voltage > comparator_max_voltage)
          comparator_max_voltage = tmp_result.max_voltage;
        
        if (freq_meas_calibration_counter > FREQ_CALIB_CYCLES_CNT)
        {
          //Calculate trigger voltage
          float tmp_voltage = (comparator_min_voltage + comparator_max_voltage) / 2.0;
          if (tmp_voltage < FREQ_CALIB_MIN_VOLTAGE)
            freq_comparator_threshold_v = FREQ_TRIGGER_DEFAULT_V;
          else
            freq_comparator_threshold_v = tmp_voltage;
          
          freq_meter_calib_state = FREQ_METER_CALIB_DONE;
          menu_draw_frequency_meter_menu(MENU_MODE_FULL_REDRAW);
          START_TIMER(calibration_timer, FREQ_METER_DONE_WAIT_DELAY);
          return;
        }
        
        data_processing_start_new_capture();//start to measure voltage
        return;
      }
    }
    else if (freq_meter_calib_state == FREQ_METER_CALIB_DONE)
    {
      if (TIMER_ELAPSED(calibration_timer))
      {
        freq_meter_calib_state = FREQ_METER_CALIB_IDLE;
        menu_draw_frequency_meter_menu(MENU_MODE_FULL_REDRAW);
      }
    }
  }
  prev_menu_mode = freq_meter_calib_state;
}
