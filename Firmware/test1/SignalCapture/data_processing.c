

/* Includes ------------------------------------------------------------------*/
#include "data_processing.h"
#include "adc_controlling.h"
#include "generator_timer.h"
#include "comparator_handling.h"
#include "mode_controlling.h"
#include "baud_meter.h"
#include "freq_measurement.h"
#include "slow_scope.h"
#include "menu_selector.h"
#include "nvram.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/

// Number of ADC points in "GENERATOR_TIMER" period
#define DATA_PROC_LOGIC_PROBE_SAMPLE_PERIOD     (DATA_PROC_LOW_SAMPLE_RATE / GENERATOR_TIMER_FREQ)

// Half of "DATA_PROC_LOGIC_PROBE_SAMPLE_PERIOD" - time of SAME logic level
#define DATA_PROC_LOGIC_PROBE_SAMPLE_HPERIOD    (DATA_PROC_LOGIC_PROBE_SAMPLE_PERIOD / 2)

//Number of HPERIODS in all captured data
#define DATA_PROC_LOGIC_PROBE_HPERIODS_NUM      (MAIN_ADC_CAPTURED_POINTS / DATA_PROC_LOGIC_PROBE_SAMPLE_HPERIOD)

//in ADC1 points
#define DATA_PROC_LOGIC_PROBE_BIG_DIFF_THRESHOLD 130

//in ADC1 points
#define DATA_PROC_LOGIC_PROBE_LOW_DIFF_THRESHOLD 30

//in ADC1 points
#define DATA_PROC_LOGIC_PROBE_HIGH_STATE_THRESHOLD 260 //~2V
#define DATA_PROC_LOGIC_PROBE_LOW_STATE_THRESHOLD  130 //~1V

//If peak-peak voltage is bigger than this voltage that mean tha it is pulsed voltage
#define DATA_PROC_LOGIC_PROBE_PEAK_THRESHOLD       0.5f //V

#define DATA_PROC_LOGIC_PROBE_ANALYSE_LENGTH    \
 (DATA_PROC_LOGIC_PROBE_SAMPLE_HPERIOD - DATA_PROC_LOGIC_PROBE_START_OFFSET * 2) //start and end
   
//****************************************

// If voltage is higher this threshold, ADC1 result is used, [V]
#define DATA_PROC_FINE_VOLTAGE_THRESHOLD        2.8f

// If (max-min) voltage less than this value, so it is stable
#define DATA_PROC_STABLE_ANALYSE_THRESHOLD      0.15f //V
   
// Min voltage for adc calibration
#define DATA_PROC_MIN_ADC_CALIB_VOLTAGE         3.0f //V

#define DATA_PROC_MIN_ADC_CALIB_FIFO_SIZE       10


/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
data_processing_state_t data_processing_state = PROCESSING_IDLE;

// Odd items - high state of "generator timer", even - low state
uint16_t logic_probe_results[DATA_PROC_LOGIC_PROBE_HPERIODS_NUM];

// Last input state, detected by logic probe
signal_state_t logic_probe_signal_state;

// Input division coefficient
float data_processing_main_div = ADC_MAIN_DIVIDER;

// Converting imput voltage to ADC2 voltage (divided and amplified)
float data_processing_amp_div = ADC_MAIN_AMP_DIVIDER;

// Curent voltage
float voltmeter_voltage = 0.0f;

//State of ADC calibration
adc_calibration_state_t data_processing_adc_calib_state = ADC_CALIB_DISPLAY_MSG1;
uint8_t data_processing_adc_calib_running = 0;
uint32_t data_processing_adc_calib_timer = 0;
uint16_t data_processing_adc_calib_fifo[DATA_PROC_MIN_ADC_CALIB_FIFO_SIZE];
uint8_t data_processing_adc_calib_fifo_pos = 0;
float data_processing_adc_calib_voltage = 0.0;//volts

extern nvram_data_t nvram_data;
extern menu_mode_t main_menu_mode;
extern volatile cap_status_type adc_capture_status;
extern volatile uint16_t adc_raw_buffer0[ADC_BUFFER_SIZE];

/* Private function prototypes -----------------------------------------------*/
void data_processing_logic_probe_handler(void);
void data_processing_process_logic_probe_data(void);
uint16_t data_processing_calc_adc_average(uint16_t* adc_buffer, uint16_t length);
void data_processing_voltmeter_handler(void);
void data_processing_process_voltmeter_data(void);
uint16_t data_processing_calc_peak_peak(uint16_t* adc_buffer, uint16_t length);
float data_processing_adc_to_voltage(uint16_t adc1, uint16_t adc2);
uint16_t data_processing_calculate_edges(uint16_t* adc_buffer, uint16_t length, float threshold_v);
void data_processing_adc_calibraion_mode(void);
void data_processing_process_adc_calibration_data(void);
uint8_t data_processing_process_adc_calibraion_fifo(void);
void data_processing_adc_calibration_add_to_fifo(uint16_t new_value);

/* Private functions ---------------------------------------------------------*/

void data_processing_init(void)
{
  //use calibration coefficient from nvram
  data_processing_main_div = ADC_MAIN_DIVIDER * nvram_data.div_a_coef;
}

// Remove sampling offset from ADC1 results
void data_processing_correct_raw_data(uint16_t zero_offset)
{
  float a_coef = (float)zero_offset / (float)(MAIN_ADC_HALF_VALUE - zero_offset);
  float b_coef = -a_coef * (float)MAIN_ADC_HALF_VALUE;
  
  for (uint16_t i = 0; i < MAIN_ADC_CAPTURED_POINTS; i++)
  {
    uint16_t raw_value = adc_raw_buffer0[i*2];
    float tmp_value = a_coef * raw_value + b_coef;//correction offset
    tmp_value = (float)raw_value + tmp_value;
    if (tmp_value < 0.0f)
      tmp_value = 0.0;
    adc_raw_buffer0[i*2] = (uint16_t)tmp_value;
  }
}

//Return ADC1 zero offset in ADC points for current sample rate
uint16_t data_processing_get_adc_offset(void)
{
  if (adc_current_sample_rate == DATA_PROC_LOW_SAMPLE_RATE)
    return 11;//todo
  else if (adc_current_sample_rate == DATA_PROC_SAMPLE_RATE_200K)
    return 7;//todo
  else if (adc_current_sample_rate == DATA_PROC_SAMPLE_RATE_2M)
    return 7;//todo
  
  return 0xFFFF;//error
}


// This function must be called when "main_menu_mode" is changed
// Switch capture mode
void data_processing_main_mode_changed(void)
{
  data_processing_state = PROCESSING_IDLE;
  if (main_menu_mode == MENU_MODE_LOGIC_PROBE)
  {
    adc_set_sample_rate(DATA_PROC_LOW_SAMPLE_RATE);
    generator_timer_activate_gpio();
  }
  else
  {
    generator_timer_deactivate_gpio();
  }
  
  if (main_menu_mode == MENU_MODE_VOLTMETER)
  {
    adc_set_sample_rate(DATA_PROC_LOW_SAMPLE_RATE);
  }
  else if (main_menu_mode == MENU_MODE_FREQUENCY_METER)// used for trigger calibration
  {
    adc_set_sample_rate(DATA_PROC_LOW_SAMPLE_RATE);
  }
  else if (main_menu_mode == MENU_MODE_BAUD_METER)
  {
    adc_set_sample_rate(DATA_PROC_SAMPLE_RATE_200K);
  }
  
  //addition processing for SLOW_SCOPE mode
  slow_scope_processing_main_mode_changed();
  baud_meter_processing_main_mode_changed();
  //comparator_processing_main_mode_changed();
  freq_measurement_main_mode_changed();
  data_processing_adc_calib_running = 0;//reset
}

// Controlling data sampling and processing - called every 10 ms
void data_processing_handler(void)
{
  switch (main_menu_mode)
  {
    case MENU_MODE_LOGIC_PROBE:
      data_processing_logic_probe_handler();
    break;
    
    case MENU_MODE_VOLTMETER:
      data_processing_voltmeter_handler();
    break;
    
    case MENU_MODE_FREQUENCY_METER:
      //comparator_processing_handler();
      freq_measurement_processing_handler();
      data_processing_voltmeter_handler();//used for trigger calibration
    break;
    
    case MENU_MODE_BAUD_METER:
      baud_meter_processing_handler();
    break;
    
    case MENU_MODE_SLOW_SCOPE:
      slow_scope_processing_handler();
    break;
    
    case MENU_SELECTOR://some data handling must be done in selected menu subitem
      if (menu_selector_adc_calib_running())
        data_processing_adc_calibraion_mode();
    break;

    
    default: break;
  }
}

void data_processing_start_new_capture(void)
{
  data_processing_state = PROCESSING_IDLE;
}

//*****************************************************************************

// Data sampling and processing for "logic probe" mode
void data_processing_logic_probe_handler(void)
{
  if (data_processing_state == PROCESSING_IDLE)
  {
    generator_timer_start();
    adc_capture_start();
    data_processing_state = PROCESSING_CAPTURE_RUNNING;
  }
  else if (data_processing_state == PROCESSING_CAPTURE_RUNNING)
  {
    if (adc_capture_status == CAPTURE_DONE)
    {
      data_processing_state = PROCESSING_DATA;
      data_processing_correct_raw_data(data_processing_get_adc_offset());
      data_processing_process_logic_probe_data();
      data_processing_state = PROCESSING_DATA_DONE;
    }
  }
}

//Process data captured by ADC1
void data_processing_process_logic_probe_data(void)
{
  uint8_t i;
  
  uint16_t peak_threshold = 
    data_processing_volt_to_points(DATA_PROC_LOGIC_PROBE_PEAK_THRESHOLD);

  for (i = 0; i < DATA_PROC_LOGIC_PROBE_HPERIODS_NUM; i++)
  {
    uint16_t start = (DATA_PROC_LOGIC_PROBE_SAMPLE_HPERIOD * i + DATA_PROC_LOGIC_PROBE_START_OFFSET) * 2;//adc1
    
    logic_probe_results[i] = data_processing_calc_adc_average(
      (uint16_t*)&adc_raw_buffer0[start], DATA_PROC_LOGIC_PROBE_ANALYSE_LENGTH);
    
    //Calculate pulsation of voltage during HALF of period
    uint16_t peak_result = data_processing_calc_peak_peak(
      (uint16_t*)&adc_raw_buffer0[start], DATA_PROC_LOGIC_PROBE_ANALYSE_LENGTH);
    
    if (peak_result > peak_threshold)
    {
      logic_probe_signal_state = SIGNAL_TYPE_PULSED_STATE;
      return;
    }
  }
  
  int16_t diff_results[DATA_PROC_LOGIC_PROBE_HPERIODS_NUM / 2];
  uint8_t big_diff_cnt = 0;
  uint8_t low_diff_cnt = 0;
  
  for (i = 0; i < DATA_PROC_LOGIC_PROBE_HPERIODS_NUM; i+= 2)
  {
    diff_results[i/2] = logic_probe_results[i] - logic_probe_results[i + 1];
    
    //Big difference mean that input is floating
    if (diff_results[i/2] > DATA_PROC_LOGIC_PROBE_BIG_DIFF_THRESHOLD)
      big_diff_cnt++;
    
    //Small difference mean than input is stable
    if (diff_results[i/2] < DATA_PROC_LOGIC_PROBE_LOW_DIFF_THRESHOLD)
      low_diff_cnt++;
  }
  
  if (big_diff_cnt == (DATA_PROC_LOGIC_PROBE_HPERIODS_NUM / 2))
    logic_probe_signal_state = SIGNAL_TYPE_Z_STATE;
  else if (low_diff_cnt == (DATA_PROC_LOGIC_PROBE_HPERIODS_NUM / 2)) //satble input
  {
    //Signal is stable all the time - mean strong external signal
    if (logic_probe_results[0] > DATA_PROC_LOGIC_PROBE_HIGH_STATE_THRESHOLD)
    {
      logic_probe_signal_state = SIGNAL_TYPE_HIGH_STATE;
      data_processing_process_voltmeter_data();
    }
    else if (logic_probe_results[0] < DATA_PROC_LOGIC_PROBE_LOW_STATE_THRESHOLD)
    {
      logic_probe_signal_state = SIGNAL_TYPE_LOW_STATE;
      data_processing_process_voltmeter_data();
    }
    else
    {
      logic_probe_signal_state = SIGNAL_TYPE_UNKNOWN_STATE;
      data_processing_process_voltmeter_data();
    }
  }
  else
  {
    logic_probe_signal_state = SIGNAL_TYPE_UNKNOWN_STATE;
    data_processing_process_voltmeter_data();
  }
}

//*****************************************************************************

// Data sampling and processing for "voltmeter" mode
void data_processing_voltmeter_handler(void)
{
  if (data_processing_state == PROCESSING_IDLE)
  {
    adc_capture_start();
    data_processing_state = PROCESSING_CAPTURE_RUNNING;
  }
  else if (data_processing_state == PROCESSING_CAPTURE_RUNNING)
  {
    if (adc_capture_status == CAPTURE_DONE) //DMA transfer is done
    {
      data_processing_state = PROCESSING_DATA;
      data_processing_correct_raw_data(data_processing_get_adc_offset());
      data_processing_process_voltmeter_data();
      data_processing_state = PROCESSING_DATA_DONE;
    }
  }
}

//Process data captured by ADC1 and ADC2
void data_processing_process_voltmeter_data(void)
{
  uint16_t adc1_result = data_processing_calc_adc_average( //divider - coarse
      (uint16_t*)&adc_raw_buffer0[6], (MAIN_ADC_CAPTURED_POINTS - 3));
  uint16_t adc2_result = data_processing_calc_adc_average( //opamp - fine
        (uint16_t*)&adc_raw_buffer0[7], (MAIN_ADC_CAPTURED_POINTS - 3));
  
  voltmeter_voltage = data_processing_adc_to_voltage(adc1_result, adc2_result);
}

//*****************************************************************************

void data_processing_adc_calibraion_mode(void)
{
  if (data_processing_adc_calib_running == 0)
  {
    //first start
    data_processing_adc_calib_running = 1;
    START_TIMER(data_processing_adc_calib_timer, 1500);
    return;
  }
  
  switch (data_processing_adc_calib_state)
  {
    case ADC_CALIB_DISPLAY_MSG1:
      if (TIMER_ELAPSED(data_processing_adc_calib_timer))
      {
        //time to start ADC capture
        data_processing_adc_calib_state = ADC_CALIB_MEASURE1;
        adc_set_sample_rate(DATA_PROC_LOW_SAMPLE_RATE);
        adc_capture_start();
      }
    break;
    
    case ADC_CALIB_MEASURE1: //measuring average ext voltage at ADC1
      if (adc_capture_status == CAPTURE_DONE)
      {
        data_processing_correct_raw_data(data_processing_get_adc_offset());
        data_processing_process_adc_calibration_data();
        adc_capture_start();
      }
      break;
      
    case ADC_CALIB_DISPLAY_CALIB:
      data_processing_process_adc_calibraion_fifo();//used for recalculating "data_processing_adc_calib_voltage"
      break;
  }
}

void data_processing_process_adc_calibration_data(void)
{
  if (data_processing_adc_calib_state == ADC_CALIB_MEASURE1)
  {
    uint16_t adc1_result = data_processing_calc_adc_average( //divider - coarse
      (uint16_t*)&adc_raw_buffer0[6], (MAIN_ADC_CAPTURED_POINTS - 3));
    
    //check if the captured signal is suitable
    float tmp_voltage = data_processing_adc_to_voltage(adc1_result, 0);
    if (tmp_voltage > DATA_PROC_MIN_ADC_CALIB_VOLTAGE) //voltage is high enought
    {
      adc_processed_data_t tmp_result = data_processing_extended(
        (uint16_t*)&adc_raw_buffer0[6], (MAIN_ADC_CAPTURED_POINTS - 3));
      if (tmp_result.signal_type != ADC_SIGNAL_TYPE_STABLE)
      {
        tmp_voltage = 0.0f;
      }
      //add raw ADC1 value to fifo
      data_processing_adc_calibration_add_to_fifo(adc1_result);
      
      if (data_processing_process_adc_calibraion_fifo())
      {
        //Get stable voltage value
        data_processing_adc_calib_state = ADC_CALIB_DISPLAY_CALIB;
      }
    }
    else
    {
      data_processing_adc_calibration_add_to_fifo(adc1_result);
    }
  }
}

void data_processing_adc_calibration_add_to_fifo(uint16_t new_value)
{
  data_processing_adc_calib_fifo[data_processing_adc_calib_fifo_pos] = new_value;
  data_processing_adc_calib_fifo_pos++;
  if (data_processing_adc_calib_fifo_pos >= DATA_PROC_MIN_ADC_CALIB_FIFO_SIZE)
    data_processing_adc_calib_fifo_pos = 0;
}

uint8_t data_processing_process_adc_calibraion_fifo(void)
{
  float tmp_voltage = 
      data_processing_adc_to_voltage(data_processing_adc_calib_fifo[0], 0);
  float min = tmp_voltage;
  float max = tmp_voltage;
  float summ = 0.0f;
  for (uint8_t i = 0; i < DATA_PROC_MIN_ADC_CALIB_FIFO_SIZE; i++)
  {
    tmp_voltage = 
      data_processing_adc_to_voltage(data_processing_adc_calib_fifo[i], 0);
    if (tmp_voltage > max)
      max = tmp_voltage;
    else if (tmp_voltage < min)
      min = tmp_voltage;
    summ+= tmp_voltage;
  }
  float diff = max - min;
  if (diff < DATA_PROC_STABLE_ANALYSE_THRESHOLD)
  {
    data_processing_adc_calib_voltage = summ / DATA_PROC_MIN_ADC_CALIB_FIFO_SIZE;
    return 1;
  }
  return 0;
}

//*****************************************************************************

// adc1 - raw data from ADC1 - divided
// adc2 - raw data from ADC2 - OPAMP
// return - voltage in volts
float data_processing_adc_to_voltage(uint16_t adc1, uint16_t adc2)
{  
  if (adc1 > data_processing_volt_to_points(DATA_PROC_FINE_VOLTAGE_THRESHOLD))
  {
    //High voltage - ADC1 result must be used
    
    float adc1_float_result = 
        (float)adc1 * (float)MCU_VREF * 
        data_processing_main_div / (float)MAIN_ADC_MAX_VALUE;
    
    return adc1_float_result;
  }
  else
  {
    //Low voltage - ADC2 result must be used
    float adc2_float_result = 
        (float)adc2 * (float)MCU_VREF * 
        data_processing_amp_div / (float)MAIN_ADC_MAX_VALUE;
    
    return adc2_float_result;
  }
}

// Calculate average value from RAW adc data
// length - number of analysed points
// Return - raw voltage
uint16_t data_processing_calc_adc_average(uint16_t* adc_buffer, uint16_t length)
{
  uint16_t i;
  if (length == 0)
    return 0;
  
  uint32_t summ = 0;
  for (i = 0; i < (length * 2); i+= 2)
  {
    summ+= adc_buffer[i];// data from two ADC's alternates
  }
  return (uint16_t)(summ / length);
}

// Calculate peak-peak value from RAW adc data
// length - number of analysed points
uint16_t data_processing_calc_peak_peak(uint16_t* adc_buffer, uint16_t length)
{
  uint16_t i;
  if (length == 0)
    return 0;
  
  uint16_t min = adc_buffer[0];
  uint16_t max = adc_buffer[0];
  
  for (i = 0; i < (length * 2); i+= 2)
  {
    if (adc_buffer[i] > max)
      max = adc_buffer[i];
    if (adc_buffer[i] < min)
      min = adc_buffer[i];
  }
  return max-min;
}

//analyse signal - get min, max and edge statictics
//length - size in samples
adc_processed_data_t data_processing_extended(uint16_t* adc_buffer, uint16_t length)
{
  uint16_t i;
  adc_processed_data_t result = {0.0f, 0.0f, ADC_SIGNAL_TYPE_STABLE};
  if (length == 0)
    return result;
  
  uint16_t min = adc_buffer[0];
  uint16_t max = adc_buffer[0];
  uint16_t min_pos = 0;
  uint16_t max_pos = 0;
  
  //Get min and max aw values
  for (i = 0; i < (length * 2); i+= 2)
  {
    if (adc_buffer[i] > max)
    {
      max = adc_buffer[i];
      max_pos = i;
    }
    if (adc_buffer[i] < min)
    {
      min = adc_buffer[i];
      min_pos = i;
    }
  }
  
  result.max_voltage = data_processing_adc_to_voltage(
    adc_buffer[max_pos], adc_buffer[max_pos + 1]);
  
  result.min_voltage = data_processing_adc_to_voltage(
    adc_buffer[min_pos], adc_buffer[min_pos + 1]);
  
  result.end_voltage = data_processing_adc_to_voltage(
    adc_buffer[length * 2 - 2], adc_buffer[length * 2 - 1]);
  
  float tmp_diff = result.max_voltage - result.min_voltage;
  if (tmp_diff < DATA_PROC_STABLE_ANALYSE_THRESHOLD)
  {
    return result;//Voltage is Stable
  }
  
  float threshold_voltage = (result.max_voltage + result.min_voltage) / 2.0f;
  uint16_t edges_cnt = 
    data_processing_calculate_edges(adc_buffer, length, threshold_voltage);
  if ((edges_cnt == 1) || (edges_cnt == 2))
    result.signal_type = ADC_SIGNAL_TYPE_SINGLE;
  else
    result.signal_type = ADC_SIGNAL_TYPE_MULTI;
  return result;
}

//Calculate number of signal edges
//threshold_v - voltage in volts
uint16_t data_processing_calculate_edges(
  uint16_t* adc_buffer, uint16_t length, float threshold_v)
{
  if (length == 0)
    return 0;
  
  uint16_t raw_threshold = data_processing_volt_to_points(threshold_v);//adc1 raw value
  
  uint16_t i;
  uint16_t counter = 0;
  for (i = 2; i < (length * 2); i+= 2)
  {
    if ((adc_buffer[i - 2] >= raw_threshold) && (adc_buffer[i] < raw_threshold))
      counter++;
    else if ((adc_buffer[i - 2] < raw_threshold) && (adc_buffer[i] >= raw_threshold))
      counter++;
  }
  return counter;
}

//Convert voltage (probe input 0 - 30V) to ADC1 points
uint16_t data_processing_volt_to_points(float voltage)
{
  if (voltage < 0.0f)
    return 0;
  
  //get divided voltage -> ADC1
  float tmp_val = (voltage / data_processing_main_div);
  
  tmp_val = tmp_val * (float)MAIN_ADC_MAX_VALUE / (float)MCU_VREF;
  if (tmp_val > (float)MAIN_ADC_MAX_VALUE)
    return MAIN_ADC_MAX_VALUE;
  
  return (uint16_t)tmp_val;
}
