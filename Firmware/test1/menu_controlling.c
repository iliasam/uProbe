/* Includes ------------------------------------------------------------------*/
#include "menu_controlling.h"
#include "display_functions.h"
#include "data_processing.h"
#include "string.h"
#include "stdio.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
menu_mode_t main_menu_mode = MENU_MODE_LOGIC_PROBE;

/* Private function prototypes -----------------------------------------------*/
void menu_main_switch_to_next_mode(void);
void menu_draw_logic_probe_menu(menu_draw_type_t draw_type);
void menu_draw_voltmeter_menu(menu_draw_type_t draw_type);
void menu_print_current_voltage(char* str);
void menu_draw_frequency_meter_menu(menu_draw_type_t draw_type);

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
void menu_draw_frequency_meter_menu(menu_draw_type_t draw_type)
{
  if (draw_type == MENU_MODE_FULL_REDRAW)
  {
    display_clear_framebuffer();
    display_draw_string("FREQUENCY MEASURE", 0, 0, FONT_SIZE_8, 0);
    display_update();
  }
  else //PARTIAL update
  {
  }
}

//*****************************************************************************

void menu_print_current_voltage(char* str)
{
  if (voltmeter_voltage < 10.0f)
    sprintf(str, "  %.02fV", voltmeter_voltage);
  else
    sprintf(str, " %.02fV", voltmeter_voltage);
}

    /*
    char tmp_str[32];
    //sprintf(tmp_str, "TEST-%d", counter);
    
    //adc_capture_start();
    //while (adc_capture_status != CAPTURE_DONE) {}
    //power_controlling_meas_battery_voltage();
    
    sprintf(tmp_str, "BATT=%.2f V\n", battery_voltage);
    display_draw_string(tmp_str, 0, 0, FONT_SIZE_8, 0);
    
    sprintf(tmp_str, "ADC1-%d    ", adc_raw_buffer0[0]);
    display_draw_string(tmp_str, 0, 12, FONT_SIZE_11, 0);
    
    sprintf(tmp_str, "ADC2-%d    ", adc_raw_buffer0[1]);
    display_draw_string(tmp_str, 0, 24, FONT_SIZE_11, 0);
    
    sprintf(tmp_str, "TIME-%d    ", ms_tick);
    display_draw_string(tmp_str, 0, 36, FONT_SIZE_8, 0);
    */

  
  
  //display_draw_string("TEST - 1234", 0, 40, FONT_SIZE_6, 0);
  
  //display_update();

