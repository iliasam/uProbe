#ifndef __BAUD_METER_H
#define __BAUD_METER_H

#include "mode_controlling.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  BAUD_PROCESSING_IDLE = 0,
  BAUD_PROCESSING_CAPTURE_FAST_RUNNING,
} baud_meter_processing_state_t;

extern uint32_t baud_meter_current_rounded_baud;

/* Exported functions ------------------------------------------------------- */
void baud_meter_processing_main_mode_changed(void);
void baud_meter_processing_handler(void);


#endif 

