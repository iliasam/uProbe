/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMPARATOR_HANDLING_H
#define __COMPARATOR_HANDLING_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f30x.h"
#include "config.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  COMP_PROCESSING_IDLE = 0,
  COMP_PROCESSING_CAPTURE1_RUNNING,
  COMP_PROCESSING_CAPTURE2_RUNNING,
  COMP_PROCESSING_DATA,
  COMP_PROCESSING_DATA_DONE
} comp_processing_state_t;

#define USE_INTERUPT_MODE               (1)
#define USE_NO_EVENTS_COMP              (2)

//Callback function - called when interrupt from comparator happens
typedef void (*comp_interrupt_callback_t)(void);

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern uint32_t comparator_calc_frequency;
extern comp_processing_state_t comp_processing_state;
extern float comparator_threshold_v;

//Values calculated during trigger voltage calibration
extern float comparator_min_voltage;
extern float comparator_max_voltage;

void dac_init(void);
void comparator_init(uint8_t interrupt_mode);
void comparator_switch_to_filter(void);
void comparator_start_timer(void);
void comparator_processing_handler(void);
void comparator_start_freq_capture(void);
void comparator_processing_main_mode_changed(void);
void comparator_change_threshold_voltage(float value);
void comparator_start_wait_interrupt(comp_interrupt_callback_t callback_func);
void comparator_init(uint8_t interrupt_mode);

void comparator_set_threshold(float voltage);

#endif /* __COMPARATOR_HANDLING_H */
