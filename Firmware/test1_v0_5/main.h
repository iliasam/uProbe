/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f30x.h"

/* Exported types ------------------------------------------------------------*/
extern volatile uint32_t ms_tick;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define START_TIMER(x, duration)  (x = (ms_tick + duration))
#define TIMER_ELAPSED(x)  ((ms_tick > x) ? 1 : 0)

/* Exported functions ------------------------------------------------------- */

#endif /* __MAIN_H */
