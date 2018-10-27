#ifndef __ADC_CONTROL_H
#define __ADC_CONTROL_H

// Number of simultaneously captured points
#define MAIN_ADC_CAPTURED_POINTS        512

//Two ADC's are workng simultaneously
//Size in uint16_t elements
#define ADC_BUFFER_SIZE (uint16_t)(MAIN_ADC_CAPTURED_POINTS * 2)

typedef enum
{
  NO_CAPTURE = 0,
  CAPTURE_IN_PROCESS,
  CAPTURE_DONE
} cap_status_type;//image capture status

void adc_init_all(void);

void adc_capture_start(void);
void capture_dma_stop(void);

void adc_start_trigger_timer(void);
void init_capture_gpio(void);


void set_adc_buf(void);



#endif 

