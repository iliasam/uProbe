#ifndef __ST7735_H
#define __ST7735_H

/* Includes ------------------------------------------------------------------*/

#include "config.h"

#include <stdint.h>
#include "stm32f30x.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_gpio.h"

typedef enum
{
  COLOR_BLACK = 0,
  COLOR_WHITE = 1,
  COLOR_RED = 2,
  COLOR_GREEN = 3,
  COLOR_BLUE = 4,
  COLOR_YELLOW = 5,
  COLOR_ENUM_SIZE
} color_enum_t;

//#define INVERT_MODE				1 // Rotate display

#define DISP_DC_SET_LOW     GPIO_ResetBits(DISPLAY_DC_N_GPIO, DISPLAY_DC_N_PIN);
#define DISP_DC_SET_HIGH    GPIO_SetBits(DISPLAY_DC_N_GPIO, DISPLAY_DC_N_PIN);

#define DISP_CSN_SET_LOW    GPIO_ResetBits(DISPLAY_CS_N_GPIO, DISPLAY_CS_N_PIN);
#define DISP_CSN_SET_HIGH   GPIO_SetBits(DISPLAY_CS_N_GPIO, DISPLAY_CS_N_PIN);

#define DISP_RST_SET_LOW  GPIO_ResetBits(DISPLAY_RES_N_GPIO, DISPLAY_RES_N_PIN);
#define DISP_RST_SET_HIGH GPIO_SetBits(DISPLAY_RES_N_GPIO, DISPLAY_RES_N_PIN);

#define DISP_WIDTH                      160
#define DISP_HEIGHT                     80


void display_init_pins(void);

void display_delay(unsigned long p);	//	задержка
#define DELAY_NOP				display_delay(10);

void display_write_data8(unsigned char dat);

void display_write_cmd(unsigned char cmd);
void display_Power_Control(unsigned char vol);
void display_set_contrast_value(unsigned char value);

void display_enable_power(void);
void display_disable_power(void);

void display_init(void);
void display_clear(void);

void display_gotoxy(unsigned char x,unsigned char y);
unsigned char display_symbol_decode(unsigned char c);
void display_putch(unsigned char c);
void display_putch_inv(unsigned char c);
void display_puts(char *s);
void display_puts_inv(char *s);
void display_send_full_framebuffer(uint8_t* data);

#endif
