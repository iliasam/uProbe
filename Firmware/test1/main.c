/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "hardware.h"
#include "SSD1315.h"
#include "display_functions.h"
#include "adc_control.h"
#include "generator_timer.h"

#include <stdio.h>


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern volatile cap_status_type adc_capture_status;
extern volatile uint16_t adc_raw_buffer0[ADC_BUFFER_SIZE];

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

int main(void)
{
  hardware_init_all();
  
  display_init();
  /* Infinite loop */
  
  adc_init_all();
  //generator_timer_activate_gpio();
  //generator_timer_init();
  adc_capture_start();
  
  //generator_timer_set_high_gpio();
  
  
  display_clear();
  
  display_draw_string("TEST - 1234", 0, 0, FONT_SIZE_8, 0);
  
 // display_draw_string("TEST - 1234", 0, 12, FONT_SIZE_11, 0);
  
  display_draw_string("TEST - 1234", 0, 40, FONT_SIZE_6, 0);
  
  display_update();
  
  uint16_t counter = 0;
  while (1)
  {
    char tmp_str[32];
    //sprintf(tmp_str, "TEST-%d", counter);
    
    sprintf(tmp_str, "ADC1-%d    ", adc_raw_buffer0[0]);
    display_draw_string(tmp_str, 0, 12, FONT_SIZE_11, 0);
    
    sprintf(tmp_str, "ADC2-%d    ", adc_raw_buffer0[1]);
    display_draw_string(tmp_str, 0, 24, FONT_SIZE_11, 0);
    
    
    display_update();
    counter++;
    
    if (adc_capture_status == CAPTURE_DONE)
    {
      adc_capture_start();
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
