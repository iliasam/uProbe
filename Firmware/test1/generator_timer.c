//Timer is used to generate test signal that goes to the probe pin

/* Includes ------------------------------------------------------------------*/
#include "generator_timer.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/


/* Private functions ---------------------------------------------------------*/

// Timer is uset to generate test PWM signal
void generator_timer_init(void)
{
  RCC_APB1PeriphClockCmd(GENERATOR_TIMER_CLK, ENABLE);//Ftimer = SYSCLK
  RCC_AHBPeriphClockCmd(GENERATOR_TIMER_GPIO_CLK, ENABLE);//PWM pin

  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_OCInitTypeDef TIM_OCInitStructure;

  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);

  TIM_DeInit(GENERATOR_TIMER);

  TIM_TimeBaseStructure.TIM_Prescaler = GENERATOR_TIMER_PRESCALER - 1;
  TIM_TimeBaseStructure.TIM_Period = 
      (SystemCoreClock / GENERATOR_TIMER_PRESCALER / GENERATOR_TIMER_FREQ - 1);
  
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(GENERATOR_TIMER, &TIM_TimeBaseStructure);
  TIM_ARRPreloadConfig(GENERATOR_TIMER, ENABLE);
  
  TIM_OCStructInit(&TIM_OCInitStructure);
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = TIM_TimeBaseStructure.TIM_Period / 2;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OC4Init(GENERATOR_TIMER,&TIM_OCInitStructure);
  TIM_OC4PreloadConfig(GENERATOR_TIMER, TIM_OCPreload_Enable);
  
  TIM_CtrlPWMOutputs(GENERATOR_TIMER, ENABLE);
  
  TIM_Cmd(GENERATOR_TIMER, ENABLE);
}

//Configure termer pin as PWM out
void generator_timer_activate_gpio(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GENERATOR_TIMER_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GENERATOR_TIMER_GPIO, &GPIO_InitStructure);
  
  GPIO_PinAFConfig(GENERATOR_TIMER_GPIO, GENERATOR_TIMER_AF_SOURCE, GENERATOR_TIMER_PIN_AFIO);
  
  TIM_CtrlPWMOutputs(GENERATOR_TIMER, ENABLE);
}

void generator_timer_deactivate_gpio(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GENERATOR_TIMER_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GENERATOR_TIMER_GPIO, &GPIO_InitStructure);
  
  TIM_CtrlPWMOutputs(GENERATOR_TIMER, DISABLE);
}

// Set pin to constant high
void generator_timer_set_high_gpio(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GENERATOR_TIMER_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GENERATOR_TIMER_GPIO, &GPIO_InitStructure);
  
  TIM_CtrlPWMOutputs(GENERATOR_TIMER, DISABLE);
  GPIO_SetBits(GENERATOR_TIMER_GPIO, GENERATOR_TIMER_PIN);
}

/* Private functions ---------------------------------------------------------*/
