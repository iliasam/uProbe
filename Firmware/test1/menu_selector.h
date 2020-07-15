#ifndef __MENU_SELECTOR_H
#define __MENU_SELECTOR_H

#include "mode_controlling.h"

typedef enum
{
  MENU_SUBITEM_EXIT = 0,
  MENU_SUBITEM_INFO,
  MENU_SUBITEM_CALIBRATE,
  MENU_SUBITEM_SET_OFF_TIME,
  MENU_SUBITEM_NULL
} menu_selector_enum;


/* Exported types ------------------------------------------------------------*/
typedef struct
{
  uint8_t item_number;
  menu_selector_enum item_type;//тип подпункта
  const char     *name;
} menu_selector_item_t;

/* Exported functions ------------------------------------------------------- */
void menu_selector_draw(menu_draw_type_t draw_type);
void menu_selector_upper_button_pressed(void);
void menu_selector_upper_button_hold(void);
void menu_selector_lower_button_hold(void);
void menu_selector_lower_button_pressed(void);
uint8_t menu_selector_submenu_active(void);
uint8_t menu_selector_adc_calib_running(void);

#endif 

