
/* Includes ------------------------------------------------------------------*/
#include "config.h"
#include "mode_controlling.h"
#include "display_functions.h"
#include "power_controlling.h"
#include "nvram.h"
#include "stdio.h"
#include "data_processing.h"

#include "menu_selector.h"

/* Private typedef -----------------------------------------------------------*/
// Step of adc divider calibration in menu, Volts
#define MENU_SELECTOR_ADC_CALIB_STEP            0.01f

/* Private variables ---------------------------------------------------------*/

menu_selector_item_t menu_selector_items[] = 
{
  {1, MENU_SUBITEM_EXIT, "EXIT"},
  {2, MENU_SUBITEM_INFO, "INFO"},
  {3, MENU_SUBITEM_CALIBRATE, "CALIBRATE ADC"},
  {4, MENU_SUBITEM_SET_OFF_TIME, "SET OFF TIME"},
  {0, MENU_SUBITEM_NULL, ""}, //null item
};

menu_selector_enum menu_selector_selected = MENU_SUBITEM_EXIT;

//Setted to 1 when submenu is active
uint8_t menu_selector_submenu_flag = 0;

//Data must be saved to nvram after exiting subitem
uint8_t menu_selector_value_changed_flag = 0;

extern nvram_data_t nvram_data;
extern adc_calibration_state_t data_processing_adc_calib_state;
extern float data_processing_adc_calib_voltage;
extern float data_processing_main_div;

/* Private function prototypes -----------------------------------------------*/
void menu_selector_draw_items(void);
void menu_selector_draw_items_handle_button(void);
void menu_selector_handle_button_hold_in_main_menu(void);
void menu_selector_draw_subitems(void);
void menu_selector_exit_subitem(void);
void menu_selector_draw_info_menu(void);
void menu_selector_subitem_upper_button(void);
void menu_selector_subitem_lower_button(void);
void menu_selector_draw_set_off_time(void);
void menu_selector_draw_adc_calib_menu(void);
void menu_selector_subitem_adc_config_button_pressed(uint8_t is_upper);

/* Private functions ---------------------------------------------------------*/

void menu_selector_upper_button_pressed(void)
{
  if (menu_selector_submenu_flag == 0)
  {
    menu_selector_draw_items_handle_button();
  }
  else
  {
    menu_selector_subitem_upper_button();
  }
}

void menu_selector_upper_button_hold(void)
{
  if (menu_selector_submenu_flag == 0)
  {
    menu_selector_handle_button_hold_in_main_menu();
  }
}

void menu_selector_lower_button_pressed(void)
{
  menu_selector_subitem_lower_button();
}

//exit event
void menu_selector_lower_button_hold(void)
{
  if (menu_selector_submenu_flag == 0)
  {
    menu_main_switch_to_next_mode();//leave menu selector mode
  }
  else
  {
    menu_selector_exit_subitem();
  }
}

void menu_selector_draw(menu_draw_type_t draw_type)
{
  display_clear_framebuffer();
  
  if (menu_selector_submenu_flag == 0)
  {  
    menu_selector_draw_items();
  }
  else
  {
    menu_selector_draw_subitems();
  }
  

  display_update();
}

uint8_t menu_selector_submenu_active(void)
{
  return menu_selector_submenu_flag;
}

//Return 1 if adc calibration is running
uint8_t menu_selector_adc_calib_running(void)
{
  return (menu_selector_submenu_flag & 
    (menu_selector_selected == MENU_SUBITEM_CALIBRATE));
}

// MENU SELECTOR ###########################################################

//written for small number of elements
void menu_selector_draw_items(void)
{
  uint8_t i = 0;
  uint8_t y_pos = 10;//start in pixels
  
  display_draw_string("  SELECT MENU", 0, 0, FONT_SIZE_8, 0, COLOR_WHITE);
  while (menu_selector_items[i].item_number > 0)
  {
    display_draw_string((char*)menu_selector_items[i].name, 6, y_pos, FONT_SIZE_8, 0, COLOR_WHITE); //shifted right for displaying cursor
    
    //draw cursor
    if (menu_selector_items[i].item_type == menu_selector_selected)
      display_draw_string(">", 0, y_pos, FONT_SIZE_8, 0, COLOR_WHITE);
    else
      display_draw_string(" ", 0, y_pos, FONT_SIZE_8, 0, COLOR_WHITE);
    
    y_pos+= 9;
    i++;
  }
}

// Handle pressing upper button in main menu selector
void menu_selector_draw_items_handle_button(void)
{
  menu_selector_selected++;
  if (menu_selector_selected == MENU_SUBITEM_NULL)
    menu_selector_selected = (menu_selector_enum)0;
}

// Handle holding upper button in main menu selector
void menu_selector_handle_button_hold_in_main_menu(void)
{
  if (menu_selector_selected == MENU_SUBITEM_EXIT)
  {
    menu_main_switch_to_next_mode();
  }
  else
  {
    //Enter to submenu
    menu_selector_submenu_flag = 1;
    menu_selector_value_changed_flag = 0;
    menu_selector_draw_subitems();
    data_processing_adc_calib_state = ADC_CALIB_DISPLAY_MSG1;
  }
}

// SUBITEMS ###########################################################

void menu_selector_draw_subitems(void)
{
  switch (menu_selector_selected)
  {
    case MENU_SUBITEM_INFO:
      menu_selector_draw_info_menu();
      break;
      
    case MENU_SUBITEM_SET_OFF_TIME:
      menu_selector_draw_set_off_time();
      break;
      
    case MENU_SUBITEM_CALIBRATE: 
      menu_selector_draw_adc_calib_menu();
      break;
    
    default: break;
  }
}

void menu_selector_exit_subitem(void)
{
  menu_selector_submenu_flag = 0;
  menu_main_switch_to_next_mode();//leave menu selector mode
  if (menu_selector_value_changed_flag != 0)
  {
    nvram_save_current_settings();
    power_controlling_update_power_off_time();
  }
}

//Handle pressing upper button in subitem mode
void menu_selector_subitem_upper_button(void)
{
  switch (menu_selector_selected)
  {     
    case MENU_SUBITEM_SET_OFF_TIME:
      nvram_data.power_off_time+= 10;//add 10 sec
      menu_selector_value_changed_flag = 1;
      break;
      
    case MENU_SUBITEM_CALIBRATE:
      menu_selector_subitem_adc_config_button_pressed(1);
      break;
    
    default: break;
  }
}

//Handle pressing upper button in subitem mode
void menu_selector_subitem_lower_button(void)
{
  switch (menu_selector_selected)
  {     
    case MENU_SUBITEM_SET_OFF_TIME:
      if (nvram_data.power_off_time > 10)
        nvram_data.power_off_time-= 10;//add 10 sec
      menu_selector_value_changed_flag = 1;
      break;
      
    case MENU_SUBITEM_CALIBRATE:
      menu_selector_subitem_adc_config_button_pressed(0);
      break;
    
    default: break;
  }
}

//Handle pressing buttons in "MENU_SUBITEM_CALIBRATE"
//is_upper - upper button pressed flag
void menu_selector_subitem_adc_config_button_pressed(uint8_t is_upper)
{
  float calib_step = MENU_SELECTOR_ADC_CALIB_STEP;
  if (is_upper == 0)
    calib_step = -calib_step;//decrease current value
  
  float current_adc_val = 
    data_processing_adc_calib_voltage / data_processing_main_div;
  
  float new_coef = data_processing_main_div + calib_step / current_adc_val;
  data_processing_main_div = new_coef;
  //Calculate nvram correction coefficient
  nvram_data.div_a_coef = data_processing_main_div / ADC_MAIN_DIVIDER;
  menu_selector_value_changed_flag = 1;
}

//*****************************************************************************

void menu_selector_draw_adc_calib_menu(void)
{
  display_draw_string(" ADC CALIBRATION", 0, 0, FONT_SIZE_8, 0, COLOR_WHITE);
  
  switch (data_processing_adc_calib_state)
  {
    case ADC_CALIB_DISPLAY_MSG1:
      display_draw_string("TOUCH EXTERNAL", 0, 13, FONT_SIZE_11, 0, COLOR_WHITE);
      display_draw_string("   VOLTAGE", 0, 27, FONT_SIZE_11, 0, COLOR_WHITE);
      display_draw_string("  FOR 1 SEC", 0, 41, FONT_SIZE_11, 0, COLOR_WHITE);
      break;
      
    case ADC_CALIB_MEASURE1:
      display_draw_string("  MEASURING", 0, 13, FONT_SIZE_11, 0, COLOR_WHITE);
      display_draw_string("   VOLTAGE", 0, 27, FONT_SIZE_11, 0, COLOR_WHITE);
      break;
      
    case ADC_CALIB_DISPLAY_CALIB:
      display_draw_string("  Correct voltage:", 0, 12, FONT_SIZE_8, 0, COLOR_WHITE);
      char tmp_str[32];
      menu_print_big_voltage(tmp_str, data_processing_adc_calib_voltage);
      display_draw_string(tmp_str, 0, 24, FONT_SIZE_22, 0, COLOR_WHITE);
      break;
  }
}

//*****************************************************************************

void menu_selector_draw_info_menu(void)
{
  display_draw_string(" DEVICE INFO", 0, 0, FONT_SIZE_8, 0, COLOR_WHITE);
  display_draw_string("FW VERSION: 1.1", 0, 12, FONT_SIZE_8, 0, COLOR_WHITE);
  
  char tmp_str[32];
  float bat_voltage = power_controlling_meas_battery_voltage();
  sprintf(tmp_str, "BATT VOLT: %.02f V", bat_voltage);
  display_draw_string(tmp_str, 0, 24, FONT_SIZE_8, 0, COLOR_WHITE);
  
  display_draw_string(" by ILIASAM 2021", 0, 50, FONT_SIZE_8, 0, COLOR_WHITE);
}

//*****************************************************************************

void menu_selector_draw_set_off_time(void)
{
  display_draw_string(" SET OFF TIME", 0, 10, FONT_SIZE_11, 0, COLOR_WHITE);
  
  char tmp_str[32];
  sprintf(tmp_str, "TIME: %d s", nvram_data.power_off_time);
  display_draw_string(tmp_str, 0, 30, FONT_SIZE_11, 0, COLOR_WHITE);
}
