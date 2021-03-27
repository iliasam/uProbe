// Timer is running with MCU clock
// Comapator is generating capture events for the timer
// Time values, measured by timer, are sent to the DMA

/* Includes ------------------------------------------------------------------*/
#include "comparator_handling.h"
#include "mode_controlling.h"
#include "data_processing.h"
#include "adc_controlling.h"
#include "freq_measurement.h"
#include "hardware.h"
#include "main.h"
#include "string.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
//Max DAC value
#define COMP_DAC_MAX_VALUE              (4095)

//Full timer period ~ 0.10 sec
#define COMP_TIMER_SLOW_PRECSALER       (50)
#define COMP_TIMER_SLOW_FREQUENCY       (SystemCoreClock / COMP_TIMER_SLOW_PRECSALER)

//Full timer period ~ 8 ms
#define COMP_TIMER_FAST_PRECSALER       (4)
#define COMP_TIMER_FAST_FREQUENCY       (SystemCoreClock / COMP_TIMER_FAST_PRECSALER)

//Size of DMA buffer for comparator events
#define COMP_DMA_BUF_SIZE               (128)

//If period < this value so fast mode must be run
#define COMP_MIN_PERIOD_IN_SLOW_MODE    (80)

#define COMP_MIN_PERIOD_IN_FAST_MODE    (5)


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

comp_processing_state_t comp_processing_state = COMP_PROCESSING_IDLE;
extern data_processing_state_t data_processing_state;
extern volatile uint16_t adc_raw_buffer0[ADC_BUFFER_SIZE];
extern float data_processing_main_div;
extern menu_mode_t main_menu_mode;

//Flag is set in timer Update interrupt
uint8_t comp_capture_done_flag = 0;

//Comparator threshold voltage, V
float comparator_threshold_v = FREQ_TRIGGER_DEFAULT_V;

uint16_t comparator_timer_prescaler = COMP_TIMER_SLOW_PRECSALER;

//Calculated frequency, Hz
uint32_t comparator_calc_frequency = 0;

// Buffer for storing event time cptured by timer
uint16_t comparator_dma_buf[COMP_DMA_BUF_SIZE];
uint16_t comparator_dma_events = 0;

// Values calculated during trigger voltage calibration
float comparator_min_voltage = 500.0f;
float comparator_max_voltage = 0.0f;

volatile uint16_t comparator_int_counter = 0;//Interrupts counter
volatile uint32_t comparator_int_dwt_buff[COMP_INTERRUPTS_DWT_BUF_SIZE];
volatile uint8_t comparator_int_dwt_buff_full = 0;

/* Private function prototypes -----------------------------------------------*/
void comparator_set_threshold(float voltage);
void comparator_timer_init(void);
void comparator_dma_init(void);
void comparator_prepare_dma(void);
uint8_t comparator_process_slow_data(void);
void comparator_process_fast_data(void);
void comparator_start_dma(void);

void COMP_MAIN_TIM_IRQ_HANDLER(void);
void COMP_MAIN_EXTI_IRQ_HANDLER(void);

/* Private functions ---------------------------------------------------------*/

void COMP_MAIN_EXTI_IRQ_HANDLER(void)
{
  EXTI_ClearITPendingBit(COMP_MAIN_IRQ_EXTI_LINE);
  
  if (comparator_int_counter < COMP_INTERRUPTS_DWT_BUF_SIZE)
  {
    comparator_int_dwt_buff[comparator_int_counter] = hardware_dwt_get();
    comparator_int_counter++;
  }
  else
  {
    COMP_Cmd(COMP_MAIN_NAME, DISABLE);
    comparator_int_dwt_buff_full = 1;
  }
  
  
  
  //COMP_Cmd(COMP_MAIN_NAME, DISABLE);
  //if (comparator_interrupt_callback_func != NULL)
  //  comparator_interrupt_callback_func();
}

void COMP_MAIN_TIM_IRQ_HANDLER(void)
{
  if (TIM_GetITStatus(COMP_MAIN_TIM_NAME, TIM_IT_Update) == SET)
  {
    DMA_Cmd(COMP_MAIN_DMA_CH, DISABLE);
    TIM_Cmd(COMP_MAIN_TIM_NAME, DISABLE);
    TIM_ClearITPendingBit(COMP_MAIN_TIM_NAME, TIM_IT_Update);
    TIM_ITConfig(COMP_MAIN_TIM_NAME, TIM_IT_Update, DISABLE);
    comp_capture_done_flag = 1;
    comparator_dma_events = COMP_DMA_BUF_SIZE - COMP_MAIN_DMA_CH->CNDTR;
  }
}

//DAC is used as NEG IN for comparator
void dac_init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  DAC_InitTypeDef DAC_InitStructure;
  
  DAC_DeInit(DAC_NAME);
  
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_Pin = DAC_PIN;
  GPIO_Init(DAC_GPIO, &GPIO_InitStructure);
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
  DAC_StructInit(&DAC_InitStructure);
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_Buffer_Switch = DAC_BufferSwitch_Disable;
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
  DAC_Init(DAC_NAME, DAC_CHANNEL, &DAC_InitStructure);
  
  DAC_Cmd(DAC_NAME, DAC_CHANNEL, ENABLE);

  comparator_set_threshold(comparator_threshold_v);
}

void comparator_init(uint8_t interrupt_mode)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  COMP_InitTypeDef COMP_InitStructure;
  EXTI_InitTypeDef EXTI_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_Pin = COMP_CAP_PIN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(COMP_CAP_GPIO, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = COMP_PIN;
  GPIO_Init(COMP_GPIO, &GPIO_InitStructure);
  
  COMP_StructInit(&COMP_InitStructure);
  COMP_InitStructure.COMP_InvertingInput    = COMP_InvertingInput_DAC1OUT1;
  COMP_InitStructure.COMP_NonInvertingInput = COMP_NonInvertingInput_IO1;
  COMP_InitStructure.COMP_Output            = COMP_Output_TIM4IC2; //<<<<
  if (interrupt_mode == USE_NO_EVENTS_COMP)
    COMP_InitStructure.COMP_Output = COMP_Output_None;
    
  COMP_InitStructure.COMP_Mode              = COMP_Mode_HighSpeed;
  COMP_InitStructure.COMP_Hysteresis        = COMP_Hysteresis_No;
  COMP_InitStructure.COMP_OutputPol         = COMP_OutputPol_NonInverted;
  COMP_Init(COMP_MAIN_NAME, &COMP_InitStructure);
  
  COMP_Cmd(COMP_MAIN_NAME, ENABLE);
  
  if (interrupt_mode == USE_INTERUPT_MODE)
  {
    DMA_Cmd(COMP_MAIN_DMA_CH, DISABLE);
    TIM_Cmd(COMP_MAIN_TIM_NAME, DISABLE);
    TIM_DMACmd(COMP_MAIN_TIM_NAME, COMP_MAIN_TIM_DMA_SRC, DISABLE);
    COMP_Cmd(COMP_MAIN_NAME, DISABLE);
    
    EXTI_InitStructure.EXTI_Line = COMP_MAIN_IRQ_EXTI_LINE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
  }
  else if (interrupt_mode == USE_NO_EVENTS_COMP)
  {
    DMA_Cmd(COMP_MAIN_DMA_CH, DISABLE);
    TIM_Cmd(COMP_MAIN_TIM_NAME, DISABLE);
    TIM_DMACmd(COMP_MAIN_TIM_NAME, COMP_MAIN_TIM_DMA_SRC, DISABLE);
    
    EXTI_InitStructure.EXTI_Line = COMP_MAIN_IRQ_EXTI_LINE;
    EXTI_InitStructure.EXTI_LineCmd = DISABLE;
    EXTI_Init(&EXTI_InitStructure);
    
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = COMP_MAIN_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = COMP_OUT_PIN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(COMP_OUT_GPIO, &GPIO_InitStructure);
    GPIO_PinAFConfig(COMP_OUT_GPIO, COMP_OUT_AF_SRC, COMP_OUT_AFIO);
  }
  else
  {
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannel = COMP_MAIN_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);
  }
  
  comparator_timer_init();
  comparator_dma_init();
}

void comparator_timer_init(void)
{
  TIM_ICInitTypeDef TIM_ICInitStructure;
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  
  RCC_APB1PeriphClockCmd(COMP_MAIN_TIM_CLK, ENABLE);  

  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Prescaler = COMP_TIMER_SLOW_PRECSALER - 1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInit(COMP_MAIN_TIM_NAME, &TIM_TimeBaseStructure);
  TIM_SelectOnePulseMode(COMP_MAIN_TIM_NAME, TIM_OPMode_Single);
  
  TIM_ICStructInit(&TIM_ICInitStructure);
  TIM_ICInitStructure.TIM_Channel = COMP_MAIN_TIM_CH;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICFilter = 0;
  TIM_ICInit(COMP_MAIN_TIM_NAME, &TIM_ICInitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = COMP_MAIN_TIM_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  TIM_DMAConfig(COMP_MAIN_TIM_NAME, TIM_DMABase_CCR2, TIM_DMABurstLength_1Transfer);
  TIM_DMACmd(COMP_MAIN_TIM_NAME, COMP_MAIN_TIM_DMA_SRC, ENABLE); 
  //TIM_SelectCCDMA(COMP_MAIN_TIM_NAME, FunctionalState NewState)
}

void comparator_dma_init(void)
{
  DMA_InitTypeDef DMA_InitStructure;
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  
  DMA_DeInit(COMP_MAIN_DMA_CH);//master
  DMA_StructInit(&DMA_InitStructure);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&COMP_MAIN_TIM_NAME->COMP_MAIN_TIM_CH_REG;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&comparator_dma_buf[0];
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = COMP_DMA_BUF_SIZE;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(COMP_MAIN_DMA_CH, &DMA_InitStructure);
}

//Start main comparator timer
void comparator_start_timer(void)
{
  TIM_Cmd(COMP_MAIN_TIM_NAME, DISABLE);
  comp_capture_done_flag = 0;
  TIM_SetCounter(COMP_MAIN_TIM_NAME, 0);
  
  TIM_PrescalerConfig(
    COMP_MAIN_TIM_NAME, 
    (comparator_timer_prescaler - 1), 
    //TIM_PSCReloadMode_Update);
    TIM_PSCReloadMode_Immediate);
    
  TIM_ClearFlag(COMP_MAIN_TIM_NAME, TIM_FLAG_Update);
  TIM_ITConfig(COMP_MAIN_TIM_NAME, TIM_IT_Update, ENABLE);
  TIM_Cmd(COMP_MAIN_TIM_NAME, ENABLE);
}

void comparator_prepare_dma(void)
{
  DMA_Cmd(COMP_MAIN_DMA_CH, DISABLE);
  memset(comparator_dma_buf, 0, sizeof(comparator_dma_buf));
  comparator_dma_events = 0;
  COMP_MAIN_DMA_CH->CNDTR = COMP_DMA_BUF_SIZE;
  COMP_MAIN_DMA_CH->CMAR = (uint32_t)&comparator_dma_buf[0];
  DMA_ClearITPendingBit(COMP_MAIN_DMA_FLAG);
}

void comparator_start_dma(void)
{
  DMA_Cmd(COMP_MAIN_DMA_CH, ENABLE);
}

void comparator_switch_to_filter(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  //Connect COMP6_INP - capacitor to GND
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Pin = COMP_CAP_PIN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(COMP_CAP_GPIO, &GPIO_InitStructure);
  GPIO_ResetBits(COMP_CAP_GPIO, COMP_CAP_PIN);
}

//Set comparator (DAC) threshold voltage
//"voltage" - in volts
void comparator_set_threshold(float voltage)
{
  if (voltage < 0.1f)
    return;
  
  //get divided voltage -> ADC1
  float tmp_val = (voltage / data_processing_main_div);
  tmp_val = tmp_val * (float)COMP_DAC_MAX_VALUE / (float)MCU_VREF;
  if (tmp_val > (float)COMP_DAC_MAX_VALUE)
    tmp_val = (float)COMP_DAC_MAX_VALUE;
  
  DAC_SetChannel1Data(DAC_NAME, DAC_Align_12b_R, (uint16_t)tmp_val);
}

//Start measuring frequency
void comparator_start_freq_capture(void)
{
  comp_processing_state = COMP_PROCESSING_IDLE;
}

void comparator_processing_handler(void)
{
  if (comp_processing_state == COMP_PROCESSING_IDLE)
  {
    //First try in slow mode
    comparator_set_threshold(comparator_threshold_v);
    comparator_timer_prescaler = COMP_TIMER_SLOW_PRECSALER;
    comparator_prepare_dma();
    comparator_start_timer();
    comparator_start_dma();
    
    comp_processing_state = COMP_PROCESSING_CAPTURE1_RUNNING;
  }
  else if (comp_processing_state == COMP_PROCESSING_CAPTURE1_RUNNING)
  {
    if (comp_capture_done_flag != 0)
    {
      comp_processing_state = COMP_PROCESSING_DATA;
      uint8_t slow_result = comparator_process_slow_data();
      
      if (slow_result != 0)
      {
        //Try in fast mode
        comparator_timer_prescaler = COMP_TIMER_FAST_PRECSALER;
        comp_processing_state = COMP_PROCESSING_CAPTURE2_RUNNING;
        comparator_prepare_dma();
        comparator_start_timer();
        comparator_start_dma();
      }
      else
      {
        comp_processing_state = COMP_PROCESSING_DATA_DONE;
      }
    }
  }
  else if (comp_processing_state == COMP_PROCESSING_CAPTURE2_RUNNING)
  {
    if (comp_capture_done_flag != 0)
    {
      comp_processing_state = COMP_PROCESSING_DATA;
      comparator_process_fast_data();
      comp_processing_state = COMP_PROCESSING_DATA_DONE;
    }
  }
  
  //comparator_freq_meter_trigger_handling();
}



//Return 0 - slow frequency detected
//Return 1 - bad events number
//Return 2 - high speed mode needed
uint8_t comparator_process_slow_data(void)
{
  uint16_t i;
  if (comparator_dma_events < 3)
  {
    comparator_calc_frequency = 0;
    return 1;
  }
  
  //Calculate periods
  uint16_t periods[COMP_DMA_BUF_SIZE];
  memset(periods, 0, sizeof(periods));
  for (i = 2; i < comparator_dma_events; i++)
  {
    periods[i - 2] = comparator_dma_buf[i] - comparator_dma_buf[i-1];
    if (periods[i - 2] < COMP_MIN_PERIOD_IN_SLOW_MODE)
    {
      comparator_calc_frequency = 0;
      return 2;
    }
  }
  
  //Calculate average period
  uint32_t period_summ = 0;
  for (i = 0; i < (comparator_dma_events - 3); i++)
  {
    period_summ+= periods[i];
  }
  period_summ =  period_summ / (comparator_dma_events - 3);
  
  comparator_calc_frequency = COMP_TIMER_SLOW_FREQUENCY / period_summ;
  return 0;//ok
}

void comparator_process_fast_data(void)
{
  uint16_t i;
  if (comparator_dma_events < 3)
  {
    comparator_calc_frequency = 0xFFFFFFFF;
    return;
  }
  
  //Calculate periods
  uint16_t periods[COMP_DMA_BUF_SIZE];
  for (i = 2; i <= comparator_dma_events; i++)
  {
    periods[i - 2] = comparator_dma_buf[i] - comparator_dma_buf[i-1];
    if (periods[i - 2] < COMP_MIN_PERIOD_IN_FAST_MODE)
    {
      comparator_calc_frequency = 0xFFFFFFFF;
      return;
    }
  }
  
  //Calculate average period
  uint32_t period_summ = 0;
  for (i = 0; i < (comparator_dma_events - 2); i++)
  {
    period_summ+= periods[i];
  }
  period_summ =  period_summ / (comparator_dma_events - 2);
  
  comparator_calc_frequency = COMP_TIMER_FAST_FREQUENCY / period_summ;
}

void comparator_change_threshold_voltage(float value)
{
  if (value < 0.0f)
    return;
  comparator_threshold_v = value;
}

void comparator_start_wait_interrupt(void)
{
  comparator_int_counter = 0;
  comparator_int_dwt_buff_full = 0;
  
  NVIC_InitTypeDef NVIC_InitStructure;
  
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannel = COMP_MAIN_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  EXTI_ClearITPendingBit(COMP_MAIN_IRQ_EXTI_LINE);
  COMP_Cmd(COMP_MAIN_NAME, ENABLE);
  EXTI_ClearITPendingBit(COMP_MAIN_IRQ_EXTI_LINE);
    
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}


