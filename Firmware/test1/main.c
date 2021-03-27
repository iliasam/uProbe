//stm32f303cbt6
//32kb ram
//128kb flash
//32 MHz here
/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "hardware.h"
#include "display_functions.h"
#include "adc_controlling.h"
#include "generator_timer.h"
#include "comparator_handling.h"
#include "power_controlling.h"
#include "keys_controlling.h"
#include "mode_controlling.h"
#include "data_processing.h"
#include "freq_measurement.h"
#include "nvram.h"

#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern volatile float battery_voltage;
extern volatile uint32_t ms_tick;

// Variable to count 1 ms periods
uint32_t timer_1ms = 0;

// Variable to count 10 ms periods
uint32_t timer_10ms = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

int main(void)
{
  hardware_init_all();
  nvram_read_data();
  display_init();
  
  dac_init();
  adc_init_all();
  generator_timer_init();
  power_controlling_init();
  comparator_init(0);
  freq_measurement_init_timers();
  keys_init();
  display_full_clear();
  menu_main_init();
  data_processing_init();
  
  while (1)
  {
    if (TIMER_ELAPSED(timer_1ms))
    {
      START_TIMER(timer_1ms, 1);
      key_handling();
    }
    
    if (TIMER_ELAPSED(timer_10ms))
    {
      START_TIMER(timer_10ms, 10);
      
      power_controlling_handler();
      menu_redraw_display(MENU_MODE_PARTIAL_REDRAW);
      data_processing_handler();
    }
  }
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
