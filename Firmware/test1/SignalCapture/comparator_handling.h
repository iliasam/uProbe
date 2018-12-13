/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMPARATOR_HANDLING_H
#define __COMPARATOR_HANDLING_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f30x.h"
#include "config.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void dac_init(void);
void comparator_init(void);
void comparator_switch_to_filter(void);
void comparator_start_timer(void);


#endif /* __COMPARATOR_HANDLING_H */
