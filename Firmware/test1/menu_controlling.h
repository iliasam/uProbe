/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MENU_CONTROLLING_H
#define __MENU_CONTROLLING_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f30x.h"
#include "config.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  MENU_MODE_FULL_REDRAW = 0,
  MENU_MODE_PARTIAL_REDRAW,
} menu_draw_type_t;

typedef enum
{
  MENU_MODE_LOGIC_PROBE = 0,
  MENU_MODE_VOLTMETER,
  MENU_MODE_FREQUENCY_METER,
  MENU_MODE_COUNT,//LAST!
} menu_mode_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void menu_lower_button_pressed(void);
void menu_redraw_display(menu_draw_type_t draw_type);
void menu_main_init(void);

#endif /* __MENU_CONTROLLING_H */

