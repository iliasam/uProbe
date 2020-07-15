/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NVRAM_H
#define __NVRAM_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f30x.h"
#include "config.h"

/* Exported types ------------------------------------------------------------*/
typedef struct
{
  uint16_t flash_ok_flag;
  float div_a_coef;
  float div_b_coef;
  uint16_t power_off_time;//seconds
} nvram_data_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void nvram_use_dafault_settings(void);
void nvram_save_current_settings(void);
void nvram_read_data(void);

#endif /* __NVRAM_H */
