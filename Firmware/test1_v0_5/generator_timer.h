/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GENERATOR_TIMER_H
#define __GENERATOR_TIMER_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f30x.h"
#include "config.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void generator_timer_init(void);
void generator_timer_activate_gpio(void);
void generator_timer_deactivate_gpio(void);
void generator_timer_set_high_gpio(void);
void generator_timer_start(void);


#endif /* __GENERATOR_TIMER_H */
