/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DATA_PROCESSING_H
#define __DATA_PROCESSING_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f30x.h"
#include "config.h"

/* Exported types ------------------------------------------------------------*/
// Hz
#define DATA_PROC_LOW_SAMPLE_RATE               (10000)

#define DATA_PROC_SAMPLE_RATE_200K              (200000)

#define DATA_PROC_SAMPLE_RATE_2M                (2000000)

// Number of sampled points to skip
#define DATA_PROC_LOGIC_PROBE_START_OFFSET      (4)

typedef enum
{
  PROCESSING_IDLE = 0,
  PROCESSING_CAPTURE_RUNNING,
  PROCESSING_DATA,
  PROCESSING_DATA_DONE
} data_processing_state_t;

typedef enum
{
  ADC_CALIB_DISPLAY_MSG1 = 0,
  ADC_CALIB_MEASURE1,//measure ext voltage
  ADC_CALIB_DISPLAY_CALIB,
} adc_calibration_state_t;

typedef enum
{
  SIGNAL_TYPE_Z_STATE = 0,
  SIGNAL_TYPE_LOW_STATE,
  SIGNAL_TYPE_HIGH_STATE,
  SIGNAL_TYPE_PULSED_STATE,
  SIGNAL_TYPE_UNKOWN_STATE,
} signal_state_t;

typedef enum
{
  ADC_SIGNAL_TYPE_STABLE = 0,
  ADC_SIGNAL_TYPE_SINGLE, //single edge
  ADC_SIGNAL_TYPE_MULTI,
} adc_signal_state_t;

typedef struct
{
  float min_voltage;
  float max_voltage;
  float end_voltage;//voltage of the last point
  adc_signal_state_t signal_type; 
} adc_processed_data_t;


/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern data_processing_state_t data_processing_state;
extern signal_state_t logic_probe_signal_state;
extern float voltmeter_voltage;

void data_processing_init(void);
uint16_t data_processing_volt_to_points(float voltage);
void data_processing_main_mode_changed(void);
void data_processing_handler(void);
void data_processing_start_new_capture(void);

void data_processing_correct_raw_data(uint16_t zero_offset);
uint16_t data_processing_get_adc_offset(void);

adc_processed_data_t data_processing_extended(uint16_t* adc_buffer, uint16_t length);


#endif /* __DATA_PROCESSING_H */

