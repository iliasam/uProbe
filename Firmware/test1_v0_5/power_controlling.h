/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __POWER_CONTROLLING_H
#define __POWER_CONTROLLING_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f30x.h"
#include "config.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
uint8_t power_controlling_is_debug(void);
void power_controlling_init_adc(void);
void power_controlling_meas_battery_voltage(void);
void power_controlling_enter_sleep(void);
void power_controlling_handler(void);
void power_controlling_event(void);

#endif /* __POWER_CONTROLLING_H */
