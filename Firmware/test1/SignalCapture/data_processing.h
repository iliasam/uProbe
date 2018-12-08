/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DATA_PROCESSING_H
#define __DATA_PROCESSING_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f30x.h"
#include "config.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  PROCESSING_IDLE = 0,
  PROCESSING_CAPTURE_RUNNING,
  PROCESSING_DATA,
  PROCESSING_DATA_DONE
} data_processing_state_t;

typedef enum
{
  SIGNAL_TYPE_Z_STATE = 0,
  SIGNAL_TYPE_LOW_STATE,
  SIGNAL_TYPE_HIGH_STATE,
  SIGNAL_TYPE_PULSED_STATE,
  SIGNAL_TYPE_UNKOWN_STATE,
} signal_state_t;


/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern data_processing_state_t data_processing_state;
extern signal_state_t logic_probe_signal_state;
extern float voltmeter_voltage;

void data_processing_main_mode_changed(void);
void data_processing_handler(void);
void data_processing_start_new_capture(void);


#endif /* __DATA_PROCESSING_H */

