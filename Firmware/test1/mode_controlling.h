/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MODE_CONTROLLING_H
#define __MODE_CONTROLLING_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f30x.h"
#include "config.h"

/* Exported types ------------------------------------------------------------*/

// Delay in ms before calibration start
#define FREQ_METER_START_WAIT_DELAY             (2000)

// Delay in ms after calibration is done
#define FREQ_METER_DONE_WAIT_DELAY              (2000)

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
  MENU_MODE_BAUD_METER,
  MENU_MODE_SLOW_SCOPE,
  MENU_SELECTOR,
  MENU_MODE_COUNT,//LAST!
} menu_mode_t;

// Calibration state
typedef enum
{
  FREQ_METER_CALIB_IDLE = 0,
  FREQ_METER_CALIB_WAIT_START, // Wait for calibration start and dispay notification
  FREQ_METER_CALIB_CAPTURE,
  FREQ_METER_CALIB_DONE,
} freq_meter_calib_state_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void menu_main_switch_to_next_mode(void);
void menu_lower_button_pressed(void);
void menu_upper_button_pressed(void);
void menu_redraw_display(menu_draw_type_t draw_type);
void menu_draw_frequency_meter_menu(menu_draw_type_t draw_type);
void menu_main_init(void);
void menu_shift_string_right(char* str, uint8_t max_size);
void menu_upper_button_hold(void);
void menu_lower_button_hold(void);
void menu_print_big_voltage(char* str, float voltage);


#endif /* __MENU_CONTROLLING_H */

