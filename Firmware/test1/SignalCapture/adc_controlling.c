//DMA is working in Normal mode, not Circular


/* Includes ------------------------------------------------------------------*/
#include "config.h"

#include "stm32f30x.h"
#include "adc_controlling.h"
#include "hardware.h"

#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_tim.h"
#include "stm32f30x_adc.h"
#include "stm32f30x_dma.h"
#include "stm32f30x_misc.h"

// Buffer to store data from two main ADC's
// Even elements - ADC1, odd - ADC2
volatile uint16_t adc_raw_buffer0[ADC_BUFFER_SIZE];

// Pointer to ADC buffer that is written now
volatile uint16_t* data_write_adc_ptr = adc_raw_buffer0;

volatile cap_status_type adc_capture_status = NO_CAPTURE;

//Hz
uint32_t adc_current_sample_rate = 0;

/* Private function prototypes -----------------------------------------------*/
void adc_dma_init(void);
void adc_trigger_timer_init(void);
void adc_init(void);
void DMA1_Channel1_IRQHandler(void);

/* Private functions ---------------------------------------------------------*/

void adc_init_all(void)
{
  adc_init();
  adc_trigger_timer_init();
  adc_dma_init();
}

//ADC DMA interrupts
void DMA1_Channel1_IRQHandler(void)
{
  ADC_TIMER->CR1 &= (uint16_t)~TIM_CR1_CEN;//stop timer

  if(DMA_GetITStatus(DMA1_IT_TC1))
  {
    DMA_ClearITPendingBit(DMA1_IT_TC1);
    DMA_Cmd(DMA1_Channel1, DISABLE);

    adc_capture_status = CAPTURE_DONE;
  }
}


// Configure DMA and start timer
void adc_capture_start(void)
{
  DMA_Cmd(DMA1_Channel1, DISABLE);
  DMA_ClearITPendingBit(DMA1_IT_TC1);
  DMA1_Channel1->CNDTR = ADC_BUFFER_SIZE / 2;//two adc give one 32-bit "sample"
  DMA1_Channel1->CMAR = (uint32_t)data_write_adc_ptr;
  //DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
  ADC_ClearFlag(ADC1, ADC_FLAG_EOC|ADC_FLAG_OVR);
  ADC_ClearFlag(ADC2, ADC_FLAG_EOC|ADC_FLAG_OVR);
  DMA_Cmd(DMA1_Channel1, ENABLE);
  
  ADC_StartConversion(ADC2);//??
  ADC_StartConversion(ADC1);
  
  adc_start_trigger_timer();
}

void capture_dma_stop(void)
{
  TIM_Cmd(ADC_TIMER, DISABLE);
  DMA_Cmd(DMA1_Channel1, DISABLE);
  DMA_ClearITPendingBit(DMA1_IT_TC1);
}

// Start timer that triggers ADC
void adc_start_trigger_timer(void)
{
  TIM_SetCounter(ADC_TIMER, 0);
  adc_capture_status = CAPTURE_IN_PROCESS; 
  TIM_Cmd(ADC_TIMER, ENABLE);
}

//Set trigger timer frequency - Hz
void adc_set_sample_rate(uint32_t frequency)
{
  uint32_t period = SystemCoreClock / frequency;
  if (period > 0xFFFF)
  {
    while(1) {};//unexpected error
  }
  adc_current_sample_rate = frequency;
  TIM_SetAutoreload(ADC_TIMER, (period - 1));
}


//ADC trigger timer
void adc_trigger_timer_init(void)
{
  RCC_APB2PeriphClockCmd(ADC_TIMER_CLK, ENABLE);//APB2 = HCLK = 32 Mhz

  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);

  TIM_DeInit(ADC_TIMER);

  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  //TIM_TimeBaseStructure.TIM_Period = 11;//72M/12 = 6mhz
  TIM_TimeBaseStructure.TIM_Period = ADC_TIMER_PERIOD;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(ADC_TIMER, &TIM_TimeBaseStructure);

  //TRGO from timer Source
  TIM_SelectOutputTrigger(ADC_TIMER, TIM_TRGOSource_Update);
  TIM_ARRPreloadConfig(ADC_TIMER, ENABLE);
}

//ADC1 & ADC2
void adc_init(void)
{
  ADC_InitTypeDef ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_ADCCLKConfig(RCC_ADC12PLLCLK_Div1);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC12, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  
  // Configure ADC Channel 12 pin as analog input
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_InitStructure.GPIO_Pin = ADC_MAIN_IN_PIN;
  GPIO_Init(ADC_MAIN_IN_GPIO, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = ADC_OPAMP_IN_PIN;
  GPIO_Init(ADC_OPAMP_IN_GPIO, &GPIO_InitStructure);

  ADC_DeInit(ADC1);
  ADC_DeInit(ADC2);
   
  // Calibration
  ADC_VoltageRegulatorCmd(ADC1, ENABLE);
  ADC_VoltageRegulatorCmd(ADC2, ENABLE);
  dwt_delay_us(1000);
  ADC_SelectCalibrationMode(ADC1, ADC_CalibrationMode_Single);//Single input
  ADC_StartCalibration(ADC1);
  while(ADC_GetCalibrationStatus(ADC1) != RESET);

  ADC_SelectCalibrationMode(ADC2, ADC_CalibrationMode_Single);
  ADC_StartCalibration(ADC2);
  while(ADC_GetCalibrationStatus(ADC2) != RESET);
  
  // ADC Common configuration *************************************************
  ADC_CommonStructInit(&ADC_CommonInitStructure);
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_RegSimul;
  ADC_CommonInitStructure.ADC_Clock = ADC_Clock_SynClkModeDiv1;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;
  ADC_CommonInitStructure.ADC_DMAMode = ADC_DMAMode_OneShot;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = 0;
  ADC_CommonInit(ADC1, &ADC_CommonInitStructure);

  ADC_StructInit(&ADC_InitStructure);
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  
  ADC_InitStructure.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Disable;//ext trigger
  ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_TRIGGER_SOURCE;//TIM1_TRGO
  ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_RisingEdge;
    
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_OverrunMode = ADC_OverrunMode_Enable;//Importaint ??
  ADC_InitStructure.ADC_AutoInjMode = ADC_AutoInjec_Disable;
  ADC_InitStructure.ADC_NbrOfRegChannel = 1;// One channel for this ADC
  ADC_Init(ADC1, &ADC_InitStructure);
  ADC_Init(ADC2, &ADC_InitStructure);

  ADC_RegularChannelConfig(ADC1, ADC_MAIN_IN_CHANNEL, 1, ADC_SAMPLING_TIME);
  ADC_RegularChannelConfig(ADC2, ADC_OPAMP_IN_CHANNEL, 1, ADC_SAMPLING_TIME);

  ADC_DMAConfig(ADC1, ADC_DMAMode_OneShot);
  ADC_DMACmd(ADC1, ENABLE);

  ADC_Cmd(ADC1, ENABLE);
  ADC_Cmd(ADC2, ENABLE);

  while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_RDY));
  while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_RDY));

  ADC_StartConversion(ADC2);//page 383 manual
  ADC_StartConversion(ADC1);
}

//DMA is transferring 2 simultaneous samples from 2 ADC's as 32-bit word 
void adc_dma_init(void)
{
  DMA_InitTypeDef DMA_InitStructure;
  NVIC_InitTypeDef      NVIC_InitStructure;
  
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
                
  DMA_DeInit(DMA1_Channel1);//master
  DMA_StructInit(&DMA_InitStructure);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1_2->CDR;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&adc_raw_buffer0[0];
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = ADC_BUFFER_SIZE / 2;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
  
  DMA_ClearITPendingBit(DMA1_IT_TC1);

  DMA_Cmd(DMA1_Channel1, ENABLE);
  
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//highest
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
}
