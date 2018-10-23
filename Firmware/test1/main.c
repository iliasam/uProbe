/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "SSD1315.h"
#include "display_functions.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

int main(void)
{
  display_init();
  /* Infinite loop */
  
  display_clear();
  
  display_draw_string("TEST - 1234", 0, 0, FONT_SIZE_8, 0);
  
  display_draw_string("TEST - 1234", 0, 12, FONT_SIZE_11, 0);
  
  display_draw_string("TEST - 1234", 0, 32, FONT_SIZE_6, 0);
  
  display_update();
  
  while (1)
  {
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
