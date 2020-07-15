//Special framebuffer wrapper used for basic operations - text drawing

/* Includes ------------------------------------------------------------------*/
#include "display_functions.h"

/* Private variables ---------------------------------------------------------*/
uint16_t display_cursor_text_x = 0;
uint16_t display_cursor_text_y = 0;
uint8_t display_framebuffer[DISPLAY_WIDTH*DISPLAY_HEIGHT / 8];

volatile uint32_t display_update_time = 0;

extern volatile uint32_t ms_tick;

/* Private function prototypes -----------------------------------------------*/
void display_draw_char_size8(uint8_t chr, uint16_t x_start, uint16_t y_start, uint8_t flags);
void display_draw_char_size6(uint8_t chr, uint16_t x_start, uint16_t y_start, uint8_t flags);
void display_draw_char_size11(uint8_t chr, uint16_t x_start, uint16_t y_start, uint8_t flags);
void display_draw_char_size22(uint8_t chr, uint16_t x_start, uint16_t y_start, uint8_t flags);

/* Private functions ---------------------------------------------------------*/

void display_set_pixel(uint16_t x, uint16_t y)
{
  uint16_t loc_x = x + LCD_LEFT_OFFSET;
  if (loc_x > LCD_RIGHT_OFFSET)
    return;
  
  uint16_t loc_y = y / 8;//8 - size of lcd line
  uint32_t byte_pos = loc_y * DISPLAY_WIDTH + loc_x;
  uint8_t byte_val = 1 << ((y % 8));
  display_framebuffer[byte_pos]|= byte_val;
}

void display_reset_pixel(uint16_t x, uint16_t y)
{
  uint16_t loc_x = x + LCD_LEFT_OFFSET;
  if (loc_x > LCD_RIGHT_OFFSET)
    return;
  
  uint16_t loc_y = y / 8;//8 - size of lcd line
  uint32_t byte_pos = loc_y * DISPLAY_WIDTH + loc_x;
  uint8_t byte_val = 1 << ((y % 8));
  display_framebuffer[byte_pos]&= ~byte_val;
}

void display_clear_framebuffer(void)
{
  memset(display_framebuffer, 0, sizeof(display_framebuffer));
  display_cursor_text_x = 0;
  display_cursor_text_y = 0;
}

void display_full_clear(void)
{
  memset(display_framebuffer, 0, sizeof(display_framebuffer));
  display_clear();
  display_cursor_text_x = 0;
  display_cursor_text_y = 0;
}

void display_set_cursor_pos(uint16_t x, uint16_t y)
{
  display_cursor_text_x = x;
  display_cursor_text_y = y;
}

void display_update(void)
{
  uint32_t start_time = ms_tick;
  display_send_full_framebuffer(display_framebuffer);
  display_update_time = ms_tick - start_time;
}

//x, y - in pixel
//return string width
//String end is 0x00 char
uint16_t display_draw_string(char *s, uint16_t x, uint16_t y, uint8_t font_size, uint8_t flags)
{
  uint16_t width = get_font_width(font_size);
  uint8_t chr_pos = 0;
  char chr = *s;
  
  while (chr && (chr_pos < 50)) 
  {
    display_draw_char(chr, x + chr_pos * width, y, font_size, flags);
    chr_pos++;
    chr = s[chr_pos];
  }
  display_cursor_text_x = x + chr_pos*width;
  display_cursor_text_y = y;
  
  return chr_pos*width;
}

//Draw text at current cursor position
//String end is 0x00 char
uint16_t display_draw_string_cur(char *s, uint8_t font_size, uint8_t flags)
{
  uint16_t length = display_draw_string(s, display_cursor_text_x, display_cursor_text_y, font_size, flags);
  display_cursor_text_x+= length;
  if ((flags & LCD_NEW_LINE_FLAG) != 0)
  {
    display_cursor_text_y+= font_size;
    display_cursor_text_x = 0;
  }
    
  return length;
}

//x - size in pixel
//y - in pixel
void display_draw_char(uint8_t chr, uint16_t x, uint16_t y, uint8_t font_size, uint8_t flags)
{
  switch (font_size)
  {
    case FONT_SIZE_8:
    {
      display_draw_char_size8(chr, x, y, flags);
      break;
    }
    case FONT_SIZE_6:
    {
      display_draw_char_size6(chr, x, y, flags);
      break;
    }
    case FONT_SIZE_11:
    {
      display_draw_char_size11(chr, x, y, flags);
      break;
    }
    case FONT_SIZE_22:
    {
      display_draw_char_size22(chr, x, y, flags);
      break;
    }
  }
}

//x, y - size in pixel
void display_draw_char_size8(uint8_t chr, uint16_t x_start, uint16_t y_start, uint8_t flags)
{
  uint16_t x_pos, y_pos;
  
  //decoding symbol
  if (chr >= 32 && chr <= '~')
  {
    chr = chr - 32;
  } 
  else
  {
    if (chr >= 192)
      chr = chr - 97;
    else
    {
        return;
    }
      
  }
  
  for (x_pos = 0; x_pos < (FONT_SIZE_8_WIDTH); x_pos++)
  {
    for (y_pos = 0; y_pos < FONT_SIZE_8; y_pos++)
    {
      uint8_t pixel = display_font_size8[chr][x_pos] & (1<<y_pos);
      if (x_pos == (FONT_SIZE_8_WIDTH-1))
        pixel = 0;
      
      if (flags & LCD_INVERTED_FLAG) 
        if (pixel) pixel = 0; else pixel = 1;
      
      if (pixel) 
        display_set_pixel(x_start + x_pos, y_start + y_pos);
      else
        display_reset_pixel(x_start + x_pos, y_start + y_pos);
    }
  }
}

void display_draw_char_size6(uint8_t chr, uint16_t x_start, uint16_t y_start, uint8_t flags)
{
  uint16_t x_pos, y_pos;
  
  for (x_pos = 0; x_pos < (FONT_SIZE_6_WIDTH); x_pos++)
  {
    for (y_pos = 0; y_pos < FONT_SIZE_6; y_pos++)
    {
      uint8_t pixel = display_font_size6[chr][y_pos] & (1<<(3-x_pos));
      
      if (flags & LCD_INVERTED_FLAG) 
        if (pixel) pixel = 0; else pixel = 1;
      
      if (pixel) 
        display_set_pixel(x_start + x_pos, y_start + y_pos);
      else
        display_reset_pixel(x_start + x_pos, y_start + y_pos);
    }
  }
}

//x, y - size in pixel
void display_draw_char_size11(uint8_t chr, uint16_t x_start, uint16_t y_start, uint8_t flags)
{
  uint16_t x_pos, y_pos;
  
  //decoding symbol
  if (chr >= 32 && chr <= 128)
  {
    chr = chr - 32;
  }
  
  
  for (x_pos = 0; x_pos < (FONT_SIZE_11_WIDTH - 1); x_pos++)
  {
    for (y_pos = 0; y_pos < (FONT_SIZE_11-1); y_pos++)
    {
      if (display_font_size11[chr][y_pos] & (1<<(x_pos))) 
        display_set_pixel(x_start + x_pos, y_start + y_pos);
      else
        display_reset_pixel(x_start + x_pos, y_start + y_pos);
    }
  }
}

//x, y - size in pixel
void display_draw_char_size22(uint8_t chr, uint16_t x_start, uint16_t y_start, uint8_t flags)
{
  uint16_t x_pos, y_pos;
  
  //decoding symbol
  if (chr >= 32 && chr <= 128)
  {
    chr = chr - 32;
  }
  
  uint16_t start = chr * FONT_SIZE_22 * 2;
  
  for (y_pos = 0; y_pos < (FONT_SIZE_22 - 1); y_pos++)
  {
    uint16_t line_num = start + y_pos*2;
    uint16_t hor_line = (uint16_t)display_font_size22[line_num + 1] | ((uint16_t)display_font_size22[line_num] << 8);
    //uint16_t hor_line = ((uint16_t*)display_font_size22)[start + y_pos];
    for (x_pos = 0; x_pos < (FONT_SIZE_22_WIDTH - 1); x_pos++)
    {
      if (hor_line & (1 << (FONT_SIZE_22_WIDTH - x_pos - 1)))
        display_set_pixel(x_start + x_pos, y_start + y_pos);
      else
        display_reset_pixel(x_start + x_pos, y_start + y_pos);
    }
  }
  

}

//Draw black bar
void draw_caption_bar(uint8_t height)
{
  uint16_t x_pos, y_pos;
  for (x_pos = 0; x_pos <= LCD_RIGHT_OFFSET; x_pos++)
  {
    for (y_pos = 0; y_pos < height; y_pos++)
    {
        display_set_pixel(x_pos, y_pos);
    }
  }
}

//Horizontal line
void display_draw_line(uint16_t y)
{
  uint16_t x_pos;
  for (x_pos = 0; x_pos <= LCD_RIGHT_OFFSET; x_pos++)
  {
    display_set_pixel(x_pos, y);
  }
}

//Horizontal line
void display_clear_line(uint16_t y)
{
  uint16_t x_pos;
  for (x_pos = 0; x_pos <= LCD_RIGHT_OFFSET; x_pos++)
  {
    display_reset_pixel(x_pos, y);
  }
}

void display_draw_vertical_line(uint16_t x, uint16_t y1, uint16_t y2)
{
  //y1 must be less than y2
  if (y1 > y2)
  {
    uint16_t tmp = y1;
    y1 = y2;
    y2 = tmp;
  }
  
  for (uint16_t y = y1; y <= y2; y++)
  {
    display_set_pixel(x, y);
  }
}



uint16_t get_font_width(uint8_t font)
{
  switch (font)
  {
    case FONT_SIZE_6:  return FONT_SIZE_6_WIDTH;
    case FONT_SIZE_8:  return FONT_SIZE_8_WIDTH;
    case FONT_SIZE_11: return FONT_SIZE_11_WIDTH;
    case FONT_SIZE_22: return FONT_SIZE_22_WIDTH;
    default: return 5;
  }
}
