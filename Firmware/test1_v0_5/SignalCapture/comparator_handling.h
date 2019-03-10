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

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern uint32_t comparator_calc_frequency;
extern comp_processing_state_t comp_processing_state;
extern float comparator_threshold;

void dac_init(void);
void comparator_init(void);
void comparator_switch_to_filter(void);
void comparator_start_timer(void);
void comparator_processing_handler(void);
void comparator_start_freq_capture(void);


#endif /* __COMPARATOR_HANDLING_H */
