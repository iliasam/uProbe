/* Includes ------------------------------------------------------------------*/
#include "menu_controlling.h"
#include "display_functions.h"
#include "data_processing.h"
#include "comparator_handling.h"
#include "power_controlling.h"
#include "slow_scope.h"
#include "baud_meter.h"
#include "menu_selector.h"
#include "string.h"
#include "stdio.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
menu_mode_t main_menu_mode = MENU_MODE_LOGIC_PROBE;

freq_meter_calib_state_t freq_meter_calib_state = FREQ_METER_CALIB_IDLE;

/* Private function prototypes -----------------------------------------------*/
void menu_main_switch_to_next_mode(void);
void menu_draw_logic_probe_menu(menu_draw_type_t draw_type);
void menu_draw_voltmeter_menu(menu_draw_type_t draw_type);
void menu_print_current_voltage(char* str);
void menu_print_current_frequency(char* str);
void menu_draw_info_menu(menu_draw_type_t draw_type);
void menu_baud_meter_menu(menu_draw_type_t draw_type);
void menu_freq_meter_upper_button_pressed(void);
void draw_not_supportd(void);//to delete


/* Private functions ---------------------------------------------------------*/

void menu_main_init(void)
{
  data_processing_main_mode_changed();
  menu_redraw_display(MENU_MODE_FULL_REDRAW);
}

void menu_lower_button_pressed(void)
{
  // Здесь должен проверяться текущий режим, 
  // и в соответствии с ним нужно переключаться на новое меню или по другому обрабатывать кнопку
  menu_main_switch_to_next_mode();
}

void menu_upper_button_pressed(void)
{
  switch (main_menu_mode)
  {
    case MENU_MODE_FREQUENCY_METER:
      menu_freq_meter_upper_button_pressed();// Handler is in this file
    break;
    
    case MENU_MODE_SLOW_SCOPE:
      slow_scope_upper_button_pressed();
    break;
    
    default: break;
  }
}

// Switch main menu mode to next mode
void menu_main_switch_to_next_mode(void)
{
  main_menu_mode++;
  if (main_menu_mode >= MENU_MODE_COUNT)
    main_menu_mode = (menu_mode_t)0;
  
  data_processing_main_mode_changed();
  menu_redraw_display(MENU_MODE_FULL_REDRAW);
}

//Redraw display depending on menu mode
void menu_redraw_display(menu_draw_type_t draw_type)
{
  switch (main_menu_mode)
  {
    case MENU_MODE_LOGIC_PROBE:
      menu_draw_logic_probe_menu(draw_type);
    break;
    
    case MENU_MODE_VOLTMETER:
      menu_draw_voltmeter_menu(draw_type);
    break;
    
    case MENU_MODE_FREQUENCY_METER:
      menu_draw_frequency_meter_menu(draw_type);
    break;
    
    case MENU_MODE_BAUD_METER:
      menu_baud_meter_menu(draw_type);
    break;
    
    case MENU_MODE_SLOW_SCOPE:
      slow_scope_draw_menu(draw_type);
    break;
    
    case MENU_MODE_INFO:
      menu_draw_info_menu(draw_type);
    break;
    
    case MENU_SELECTOR:
      menu_selector_draw(draw_type);
    break;
    
    
    default: 
      draw_not_supportd();
      break;
  }
}

void draw_not_supportd(void) //to delete
{
  display_clear_framebuffer();
  display_draw_string("NOT SUPPORTED", 0, 0, FONT_SIZE_11, 0);
  display_update();
}


void menu_draw_logic_probe_menu(menu_draw_type_t draw_type)
{
  if (draw_type == MENU_MODE_FULL_REDRAW)
  {
    display_clear_framebuffer();
    display_draw_string("  LOGIC PROBE MODE", 0, 0, FONT_SIZE_8, 0);
    display_update();
  }
  else //PARTIAL update
  {
    if (data_processing_state == PROCESSING_DATA_DONE)
    {
      data_processing_start_new_capture();
      return;
    }
    
    char tmp_str[32];
    if (data_processing_state != PROCESSING_IDLE)
    {
      switch (logic_probe_signal_state)
      {
      case SIGNAL_TYPE_Z_STATE:
        display_draw_string("Z STATE", 0, 20, FONT_SIZE_22, 0);
        break;
      case SIGNAL_TYPE_LOW_STATE:
        display_draw_string("  LOW  ", 0, 20, FONT_SIZE_22, 0);
        break;
      case SIGNAL_TYPE_HIGH_STATE:
        display_draw_string("  HIGH ", 0, 20, FONT_SIZE_22, 0);
        break;
      case SIGNAL_TYPE_PULSED_STATE:
        display_draw_string(" PULSE ", 0, 20, FONT_SIZE_22, 0);
        break;
      case SIGNAL_TYPE_UNKOWN_STATE:
        display_draw_string("UNKNOWN", 0, 20, FONT_SIZE_22, 0);
        break;
      default: break;
      }
      
      //Draw voltage when input voltage is stable
      if ((logic_probe_signal_state == SIGNAL_TYPE_LOW_STATE) || 
          (logic_probe_signal_state == SIGNAL_TYPE_HIGH_STATE) ||
          (logic_probe_signal_state == SIGNAL_TYPE_UNKOWN_STATE))
      {
        menu_print_current_voltage(tmp_str);
        display_draw_string(tmp_str, 20, 46, FONT_SIZE_11, 0);
      }
      else
      {
        display_draw_string("        ", 20, 46, FONT_SIZE_11, 0);
      }

      display_update();
      
    }
  }
}

//*****************************************************************************
void menu_draw_voltmeter_menu(menu_draw_type_t draw_type)
{
  if (draw_type == MENU_MODE_FULL_REDRAW)
  {
    display_clear_framebuffer();
    display_draw_string("   VOLTMETER MODE", 0, 0, FONT_SIZE_8, 0);
    display_update();
  }
  else //PARTIAL update
  {
    if (data_processing_state == PROCESSING_DATA_DONE)
    {
      data_processing_start_new_capture();
      return;
    }
    
    if (data_processing_state != PROCESSING_IDLE)
    {
      char tmp_str[32];
      menu_print_current_voltage(tmp_str);
      display_draw_string(tmp_str, 0, 20, FONT_SIZE_22, 0);
      display_update();
    }
    
    //voltmeter_voltage
    
  }
}

//*****************************************************************************

void menu_draw_info_menu(menu_draw_type_t draw_type)
{
  if (draw_type == MENU_MODE_FULL_REDRAW)
  {
    display_clear_framebuffer();
    display_draw_string(" DEVICE INFO", 0, 0, FONT_SIZE_8, 0);
    display_draw_string("FW VERSION: 1.0", 0, 12, FONT_SIZE_8, 0);
    
    char tmp_str[32];
    float bat_voltage = power_controlling_meas_battery_voltage();
    sprintf(tmp_str, "BATT VOLT: %.02f V", bat_voltage);
    display_draw_string(tmp_str, 0, 24, FONT_SIZE_8, 0);
    
    display_draw_string("  by ILIASAM 2019", 0, 50, FONT_SIZE_8, 0);
    
    display_update();
  }
}

//*****************************************************************************
void menu_draw_frequency_meter_menu(menu_draw_type_t draw_type)
{
  if (draw_type == MENU_MODE_FULL_REDRAW)
  {
    display_clear_framebuffer();
    display_draw_string(" FREQUENCY MEASURE", 0, 0, FONT_SIZE_8, 0);
    
    if (freq_meter_calib_state == FREQ_METER_CALIB_WAIT_START)
    {   
      display_draw_string("  AUTO TRIGGER ", 0, 15, FONT_SIZE_11, 0);
      display_draw_string(" TOUCH TO SIGNAL", 0, 29, FONT_SIZE_11, 0);
    }
    else if (freq_meter_calib_state == FREQ_METER_CALIB_CAPTURE)
    {
      display_draw_string("  AUTO TRIGGER ", 0, 15, FONT_SIZE_11, 0);
      display_draw_string("   HOLD PROBE", 0, 29, FONT_SIZE_11, 0);
    }
    else if (freq_meter_calib_state == FREQ_METER_CALIB_DONE)
    {
      char tmp_str[32];
      sprintf(tmp_str, " MIN voltage: %.02fV", comparator_min_voltage);
      display_draw_string(tmp_str, 0, 22, FONT_SIZE_8, 0);
      sprintf(tmp_str, " MAX voltage: %.02fV", comparator_max_voltage);
      display_draw_string(tmp_str, 0, 32, FONT_SIZE_8, 0);
      sprintf(tmp_str, " TRIGGR: %.02fV", comparator_threshold);
      display_draw_string(tmp_str, 0, 42, FONT_SIZE_8, 0);
    }
    
    display_update();
  }
  else //PARTIAL update
  {
    if (comp_processing_state == COMP_PROCESSING_DATA_DONE)
    {
      char tmp_str[32];
      if (freq_meter_calib_state == FREQ_METER_CALIB_IDLE)//normal working
      {
        menu_print_current_frequency(tmp_str);
        display_draw_string(tmp_str, 0, 20, FONT_SIZE_22, 0);
        
        sprintf(tmp_str, "LEVEL: %.02f V", comparator_threshold);
        display_draw_string(tmp_str, 10, 50, FONT_SIZE_8, 0);
        
        display_update();
        
        comparator_start_freq_capture();
      }
    }
  }
}

void menu_freq_meter_upper_button_pressed(void)
{
  if (freq_meter_calib_state == FREQ_METER_CALIB_IDLE)
  {
    freq_meter_calib_state = FREQ_METER_CALIB_WAIT_START;// Wait for calibration start
    menu_draw_frequency_meter_menu(MENU_MODE_FULL_REDRAW);
  }
}

//*****************************************************************************

void menu_baud_meter_menu(menu_draw_type_t draw_type)
{
  if (draw_type == MENU_MODE_FULL_REDRAW)
  {
    display_clear_framebuffer();
    display_draw_string(" BAUDRATE MEASURE", 0, 0, FONT_SIZE_8, 0);
    display_update();
  }
  else //PARTIAL update
  {
    char tmp_str[32];
    if (baud_meter_current_rounded_baud < 100)
      sprintf(tmp_str, "  NONE  ");
    else
      sprintf(tmp_str, "%d    ", baud_meter_current_rounded_baud);
    
    display_draw_string(tmp_str, 0, 20, FONT_SIZE_22, 0);
      display_update();
  }
}
//*****************************************************************************

void menu_print_current_frequency(char* str)
{
  if (comparator_calc_frequency > 1e6)
    sprintf(str, "UNKNOWN");
  else if (comparator_calc_frequency > 99999)
    sprintf(str, "%d KHz", (comparator_calc_frequency / 1000));
  else if (comparator_calc_frequency < 1000)
    sprintf(str, " %d Hz    ", comparator_calc_frequency);
  else
    sprintf(str, "%dHz    ", comparator_calc_frequency);
}

void menu_print_current_voltage(char* str)
{
  if (voltmeter_voltage < 10.0f)
    sprintf(str, "  %.02fV", voltmeter_voltage);
  else
    sprintf(str, " %.02fV", voltmeter_voltage);
}

// Shift null-terminated string to right corner
// max_size - max size of the string
void menu_shift_string_right(char* str, uint8_t max_size)
{
  uint8_t data_size = strlen(str);//length of data in string
  
  if (data_size > max_size)
    return; //nothing to shift
  
  for (uint8_t i = 0; i < data_size; i++)
  {
    str[max_size - i - 1] = str[data_size - i - 1];
  }
  
  //Replace first symbols by spaces
  memset(str, ' ', (max_size - data_size));
}

