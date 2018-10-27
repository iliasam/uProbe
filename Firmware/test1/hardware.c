/* Includes ------------------------------------------------------------------*/
#include "hardware.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void hardware_init_rcc(void);
void hardware_dwt_init(void);
uint32_t hardware_dwt_get(void);
uint8_t hardware_dwt_comapre(int32_t tp);

/* Private functions ---------------------------------------------------------*/

void hardware_init_all(void)
{
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
  hardware_init_rcc();
  
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq (&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);
  
  hardware_dwt_init();
}

//Initialize main clock system
void hardware_init_rcc(void)
{
  //switch to HSI
  RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
  while (RCC_GetSYSCLKSource() != 0x00) {}
  RCC_DeInit();

  // PLL config 8 MHz / 2 * 8 = 32 MHz
  RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_8); 
  RCC_PLLCmd(ENABLE);
  while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}
  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
  while (RCC_GetSYSCLKSource() != 0x08) {}
  SystemCoreClockUpdate();
}

//Init DWT counter
void hardware_dwt_init(void)
{
  if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
  {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  }
}

uint32_t hardware_dwt_get(void)
{
  return DWT->CYCCNT;
}

inline uint8_t hardware_dwt_comapre(int32_t tp)
{
  return (((int32_t)hardware_dwt_get() - tp) < 0);
}

// Delay for "us"
void dwt_delay_us(uint32_t us)
{
  int32_t tp = hardware_dwt_get() + us * (SystemCoreClock / 1000000);
  while (hardware_dwt_comapre(tp));
}
