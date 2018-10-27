/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/

// BUTTONS ********************************************************************
// Button 1 - wakeup
#define BUTTON_1_GPIO           GPIOA
#define BUTTON_1_PIN            GPIO_Pin_0

// Button 2
#define BUTTON_2_GPIO           GPIOA
#define BUTTON_2_PIN            GPIO_Pin_3

// DISPLAY ********************************************************************

// Display power - active low
#define DISPLAY_PWR_N_GPIO      GPIOA
#define DISPLAY_PWR_N_PIN       GPIO_Pin_8

// Data/command line - high(data) / low(command)
#define DISPLAY_DC_N_GPIO       GPIOB
#define DISPLAY_DC_N_PIN        GPIO_Pin_3

// Display reset - active low
#define DISPLAY_RES_N_GPIO      GPIOB
#define DISPLAY_RES_N_PIN       GPIO_Pin_5

// SPI CSn
#define DISPLAY_CS_N_GPIO       GPIOB
#define DISPLAY_CS_N_PIN        GPIO_Pin_6

// SPI CLK
#define DISPLAY_CLK_GPIO        GPIOB
#define DISPLAY_CLK_PIN         GPIO_Pin_13

// SPI MOSI
#define DISPLAY_MOSI_GPIO       GPIOB
#define DISPLAY_MOSI_PIN        GPIO_Pin_15

// ADC TIMER ******************************************************************
// Timer used to trigger ADC1 and ADC2
#define ADC_TIMER_CLK           RCC_APB2Periph_TIM1

#define ADC_TIMER               TIM1

#define ADC_TIMER_PERIOD        100  //72M/12 = 6mhz

// ADC ************************************************************************

#define ADC_SAMPLING_TIME       ADC_SampleTime_1Cycles5

//Divided signal from probe -> ADC1
#define ADC_MAIN_IN_GPIO        GPIOA
#define ADC_MAIN_IN_PIN         GPIO_Pin_2
#define ADC_MAIN_IN_CHANNEL     ADC_Channel_3

//Divided and amplified signal from probe -> ADC2
#define ADC_OPAMP_IN_GPIO       GPIOA
#define ADC_OPAMP_IN_PIN        GPIO_Pin_6
#define ADC_OPAMP_IN_CHANNEL    ADC_Channel_3//OPAMP2 OUT


#define ADC_OPAMP_NAME          OPAMP_Selection_OPAMP2
#define ADC_OPAMP_GAIN          OPAMP_OPAMP_PGAGain_8
#define ADC_OPAMP_POS_INPUT     OPAMP_NonInvertingInput_IO4//PA7 for OPAMP2

// GENERATOR TIMER ************************************************************
// Timer used to generate test signal
#define GENERATOR_TIMER_CLK             RCC_APB1Periph_TIM3

#define GENERATOR_TIMER                 TIM3
#define GENERATOR_TIMER_PRESCALER       (32)
#define GENERATOR_TIMER_FREQ            100  //Hz

#define GENERATOR_TIMER_GPIO_CLK        RCC_AHBPeriph_GPIOB
#define GENERATOR_TIMER_GPIO            GPIOB
#define GENERATOR_TIMER_PIN             GPIO_Pin_7
#define GENERATOR_TIMER_AF_SOURCE       GPIO_PinSource7
#define GENERATOR_TIMER_PIN_AFIO        GPIO_AF_10




/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __MAIN_H */
