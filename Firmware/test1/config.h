/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CONFIG_H
#define __CONFIG_H

/* Includes ------------------------------------------------------------------*/

#define FW_VERSION_STRING       "FW VERSION: 1.5"

// BUTTONS ********************************************************************

// Lower button - wakeup
#define BUTTON1_GPIO            GPIOA
#define BUTTON1_PIN             GPIO_Pin_0

// Upper button
#define BUTTON2_GPIO            GPIOA
#define BUTTON2_PIN             GPIO_Pin_3

// DISPLAY ********************************************************************

#define DISPLAY_SPI_NAME        SPI2

// Display power - active low
#define DISPLAY_PWR_N_GPIO      GPIOA
#define DISPLAY_PWR_N_PIN       GPIO_Pin_8

// Data/command line - high(data) / low(command)
#define DISPLAY_DC_N_GPIO       GPIOB
#define DISPLAY_DC_N_PIN        GPIO_Pin_2

// Display reset - active low
#define DISPLAY_RES_N_GPIO      GPIOB
#define DISPLAY_RES_N_PIN       GPIO_Pin_6

// SPI CSn
#define DISPLAY_CS_N_GPIO       GPIOB
#define DISPLAY_CS_N_PIN        GPIO_Pin_5

// SPI CLK
#define DISPLAY_CLK_GPIO        GPIOB
#define DISPLAY_CLK_PIN         GPIO_Pin_13
#define DISPLAY_CLK_SRC         GPIO_PinSource13
#define DISPLAY_CLK_AFIO        GPIO_AF_5

// SPI MOSI
#define DISPLAY_MOSI_GPIO       GPIOB
#define DISPLAY_MOSI_PIN        GPIO_Pin_15
#define DISPLAY_MOSI_SRC        GPIO_PinSource15
#define DISPLAY_MOSI_AFIO       GPIO_AF_5

// ADC TIMER ******************************************************************
// Timer used to trigger ADC1 and ADC2
#define ADC_TIMER_CLK           RCC_APB2Periph_TIM8

#define ADC_TIMER               TIM8

#define ADC_TIMER_PERIOD        1000  //72M/12 = 6mhz

// ADC ************************************************************************

#define ADC_SAMPLING_TIME       ADC_SampleTime_1Cycles5
//#define ADC_SAMPLING_TIME       ADC_SampleTime_181Cycles5

#define ADC_TRIGGER_SOURCE      ADC_ExternalTrigConvEvent_7;//TIM8_TRGO

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

#define ADC_OPAMP_POS_PIN       GPIO_Pin_7
#define ADC_OPAMP_POS_GPIO      GPIOA

#define MCU_VREF                3.22f //There is some drop at RC-filter

// Input division coefficient - default value
#define ADC_MAIN_DIVIDER        (10.455f * 1.015f)

// Input division coefficient - default value
#define ADC_MAIN_AMP_DIVIDER    (ADC_MAIN_DIVIDER / 8.176f)

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

// DAC for comparator *********************************************************
#define DAC_GPIO                        GPIOA
#define DAC_PIN                         GPIO_Pin_4
#define DAC_CHANNEL                     DAC_Channel_1
#define DAC_NAME                        DAC1

// Comparator *****************************************************************
#define COMP_CAP_GPIO                   GPIOB
#define COMP_CAP_PIN                    GPIO_Pin_11 //COMP6_INP

#define COMP_GPIO                       GPIOB
#define COMP_PIN                        GPIO_Pin_0 //COMP4_INP

#define COMP_OUT_GPIO                   GPIOB
#define COMP_OUT_PIN                    GPIO_Pin_1 //COMP4_OUT
#define COMP_OUT_AF_SRC                 GPIO_PinSource1
#define COMP_OUT_AFIO                   GPIO_AF_8

//Main comparator - connected directly to the divider 
#define COMP_MAIN_NAME                  COMP_Selection_COMP4

#define COMP_MAIN_IRQ                   COMP4_5_6_IRQn
//COMP4 is internally connected to EXTI Line 30
#define COMP_MAIN_IRQ_EXTI_LINE         EXTI_Line30
#define COMP_MAIN_EXTI_IRQ_HANDLER      COMP4_5_6_IRQHandler

#define FREQ_MEAS_TIM_NAME              TIM1
#define FREQ_MEAS_TIM_CLK_INIT_F        RCC_APB2PeriphClockCmd
#define FREQ_MEAS_TIM_CLK               RCC_APB2Periph_TIM1
#define FREQ_MEAS_TIM_IRQ               TIM1_UP_TIM16_IRQn
#define FREQ_MEAS_TIM_IRQ_HANDLER       TIM1_UP_TIM16_IRQHandler

//External clock input - COMP4_OUT
#define FREQ_MEAS_TIM_ETR_GPIO          GPIOA
#define FREQ_MEAS_TIM_ETR_PIN           GPIO_Pin_12
#define FREQ_MEAS_TIM_ETR_AF_SRC        GPIO_PinSource12
#define FREQ_MEAS_TIM_ETR_AFIO          GPIO_AF_11

// POWER CONTROLLING **********************************************************
#define BATTERY_ADC_GPIO                GPIOB
#define BATTERY_ADC_PIN                 GPIO_Pin_12 //BAT_VOLT
#define BATTERY_ADC_PIN_CHANNEL         ADC_Channel_3 //ADC4

// Battery divider GND
#define BATTERY_MEAS_GND_GPIO           GPIOA
#define BATTERY_MEAS_GND_PIN            GPIO_Pin_11 //MEAS_GND

// Battery voltage divider coefficient
#define BATTERY_DIV_VALUE               (2.0f)



/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __CONFIG_H */
