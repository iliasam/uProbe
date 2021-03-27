//Comparator is used for measuring frequency (see "freq_measurement.c")
//Also is used for baudrate measurement - calculating time between interrupts

/* Includes ------------------------------------------------------------------*/
#include "comparator_handling.h"
#include "mode_controlling.h"
#include "data_processing.h"
#include "freq_measurement.h"
#include "hardware.h"
#include "main.h"
#include "string.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
//Max DAC value
#define COMP_DAC_MAX_VALUE              (4095)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern float data_processing_main_div;

//Comparator threshold voltage, V
float comparator_threshold_v = FREQ_TRIGGER_DEFAULT_V;

// Values calculated during trigger voltage calibration
float comparator_min_voltage = 500.0f;
float comparator_max_voltage = 0.0f;

volatile uint16_t comparator_int_counter = 0;//Interrupts counter
volatile uint32_t comparator_int_dwt_buff[COMP_INTERRUPTS_DWT_BUF_SIZE];
volatile uint8_t comparator_int_dwt_buff_full = 0;

/* Private function prototypes -----------------------------------------------*/
void COMP_MAIN_EXTI_IRQ_HANDLER(void);

/* Private functions ---------------------------------------------------------*/

//Comparator -> EXTI interrupt
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

  comparator_set_threshold(FREQ_TRIGGER_DEFAULT_V);
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
  COMP_InitStructure.COMP_Output = COMP_Output_None;
    
  COMP_InitStructure.COMP_Mode              = COMP_Mode_HighSpeed;
  COMP_InitStructure.COMP_Hysteresis        = COMP_Hysteresis_No;
  COMP_InitStructure.COMP_OutputPol         = COMP_OutputPol_NonInverted;
  COMP_Init(COMP_MAIN_NAME, &COMP_InitStructure);
  
  COMP_Cmd(COMP_MAIN_NAME, ENABLE);
  
  if (interrupt_mode == USE_INTERUPT_MODE)
  {
    COMP_Cmd(COMP_MAIN_NAME, DISABLE);
    
    EXTI_InitStructure.EXTI_Line = COMP_MAIN_IRQ_EXTI_LINE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
  }
  else if (interrupt_mode == USE_NO_EVENTS_COMP) //used to measure freqency
  {   
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
  
}

//Not used now
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
  
  comparator_threshold_v = voltage;
  
  //get divided voltage -> ADC1
  float tmp_val = (voltage / data_processing_main_div);
  tmp_val = tmp_val * (float)COMP_DAC_MAX_VALUE / (float)MCU_VREF;
  if (tmp_val > (float)COMP_DAC_MAX_VALUE)
    tmp_val = (float)COMP_DAC_MAX_VALUE;
  
  DAC_SetChannel1Data(DAC_NAME, DAC_Align_12b_R, (uint16_t)tmp_val);
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
