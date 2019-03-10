#ifndef __SSD1315_H
#define __SSD1315_H

/* Includes ------------------------------------------------------------------*/

#include "config.h"

#include <stdint.h>
#include "stm32f30x.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_gpio.h"

//#define INVERT_MODE				1 // Rotate display

#define DISP_DC_SET_LOW     GPIO_ResetBits(DISPLAY_DC_N_GPIO, DISPLAY_DC_N_PIN);
#define DISP_DC_SET_HIGH    GPIO_SetBits(DISPLAY_DC_N_GPIO, DISPLAY_DC_N_PIN);

#define DISP_CSN_SET_LOW    GPIO_ResetBits(DISPLAY_CS_N_GPIO, DISPLAY_CS_N_PIN);
#define DISP_CSN_SET_HIGH   GPIO_SetBits(DISPLAY_CS_N_GPIO, DISPLAY_CS_N_PIN);

#define DISP_RST_SET_LOW  GPIO_ResetBits(DISPLAY_RES_N_GPIO, DISPLAY_RES_N_PIN);
#define DISP_RST_SET_HIGH GPIO_SetBits(DISPLAY_RES_N_GPIO, DISPLAY_RES_N_PIN);

#define DISP_WIDTH                      128
#define DISP_HEIGHT                     64
#define DISP_PAGE_CNT                   8

#define DISP_DEFAULT_CONTRAST           (128)

#define SSD1315_SET_PAGE_ADDR           0xB0 // in Page Addressing Mode
#define SSD1315_SET_CONTRAST_CMD        0x81
#define SSD1315_SET_START_LINE_CMD      0x40

#define SSD1315_ALL_ON_CMD              0xA5
#define SSD1315_ALL_OFF_CMD             0xA4 //disable all on

#define SSD1315_NORMAL_MODE_CMD         0xA6
#define SSD1315_INVERSE_MODE_CMD        0xA7

void display_init_pins(void);

void display_delay(unsigned long p);	//	задержка
#define DELAY_NOP				display_delay(10);

void display_write_data(unsigned char dat);
void display_write_cmd(unsigned char cmd);
void display_set_start_line(unsigned char line);
void display_set_page_address(unsigned char page);
void display_set_column_address(unsigned char column);
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
