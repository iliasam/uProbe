/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMPARATOR_HANDLING_H
#define __COMPARATOR_HANDLING_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f30x.h"
#include "config.h"

/* Exported types ------------------------------------------------------------*/
#define USE_INTERUPT_MODE               (1)
#define USE_NO_EVENTS_COMP              (2)

#define COMP_INTERRUPTS_DWT_BUF_SIZE    (128)


/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern float comparator_threshold_v;

//Values calculated during trigger voltage calibration
extern float comparator_min_voltage;
extern float comparator_max_voltage;

void dac_init(void);
void comparator_init(uint8_t interrupt_mode);
void comparator_switch_to_filter(void);
void comparator_processing_handler(void);
void comparator_set_threshold(float voltage);
void comparator_start_wait_interrupt(void);
void comparator_init(uint8_t interrupt_mode);

#endif /* __COMPARATOR_HANDLING_H */
