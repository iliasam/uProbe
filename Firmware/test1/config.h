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



/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __MAIN_H */
