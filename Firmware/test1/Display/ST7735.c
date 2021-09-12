#include "ST7735.h"

uint8_t display_x = 0;
uint8_t display_y = 0;

//Better in RAM
uint16_t color_convert_table[COLOR_ENUM_SIZE];

void display_spi_init(void);
void LCD_SetCursor(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t  Yend);
void display_init_conv_table(void);

//***************************************************************************

// Initialize display pins
void display_init_pins(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
  
  //Common
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  
  GPIO_InitStructure.GPIO_Pin = DISPLAY_CLK_PIN;
  GPIO_Init(DISPLAY_CLK_GPIO, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = DISPLAY_MOSI_PIN;
  GPIO_Init(DISPLAY_MOSI_GPIO, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = DISPLAY_DC_N_PIN;
  GPIO_Init(DISPLAY_DC_N_GPIO, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = DISPLAY_CS_N_PIN;
  GPIO_Init(DISPLAY_CS_N_GPIO, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = DISPLAY_RES_N_PIN;
  GPIO_Init(DISPLAY_RES_N_GPIO, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = DISPLAY_PWR_N_PIN;
  GPIO_Init(DISPLAY_PWR_N_GPIO, &GPIO_InitStructure);
  
  display_spi_init();
  
  display_enable_power();
}

void display_enable_power(void)
{
  GPIO_ResetBits(DISPLAY_PWR_N_GPIO, DISPLAY_PWR_N_PIN);
}

void display_disable_power(void)
{
  //GPIO_SetBits(DISPLAY_PWR_N_GPIO, DISPLAY_PWR_N_PIN);
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//deinit pins
  
  GPIO_InitStructure.GPIO_Pin = DISPLAY_CLK_PIN;
  GPIO_Init(DISPLAY_CLK_GPIO, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = DISPLAY_MOSI_PIN;
  GPIO_Init(DISPLAY_MOSI_GPIO, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = DISPLAY_DC_N_PIN;
  GPIO_Init(DISPLAY_DC_N_GPIO, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = DISPLAY_CS_N_PIN;
  GPIO_Init(DISPLAY_CS_N_GPIO, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = DISPLAY_RES_N_PIN;
  GPIO_Init(DISPLAY_RES_N_GPIO, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = DISPLAY_PWR_N_PIN;
  GPIO_Init(DISPLAY_PWR_N_GPIO, &GPIO_InitStructure);
}


// Delay
void display_delay(unsigned long p) 
{
  unsigned long i;
  for (i = 0; i < p; i++) {}
}

// Send data to the display
void display_write_data8(unsigned char dat)
{
  DISP_CSN_SET_LOW;
  DISP_DC_SET_HIGH;
  display_delay(2);
  
  SPI_SendData8(DISPLAY_SPI_NAME, dat);
  while(SPI_I2S_GetFlagStatus(DISPLAY_SPI_NAME, SPI_I2S_FLAG_BSY) == SET) {}
  
  display_delay(2);
  DISP_CSN_SET_HIGH;
  display_delay(2);
}


// Send command to display
void display_write_cmd(unsigned char cmd) 
{
  DISP_CSN_SET_LOW;
  DISP_DC_SET_LOW;
  display_delay(2);
  
  SPI_SendData8(DISPLAY_SPI_NAME, cmd);
  while(SPI_I2S_GetFlagStatus(DISPLAY_SPI_NAME, SPI_I2S_FLAG_BSY) == SET) {}
  
  display_delay(2);
  DISP_CSN_SET_HIGH;
  display_delay(2);
}

// Set contrast value
// "value" 0-255
void display_set_contrast_value(unsigned char value) 
{
}

void display_init_conv_table(void)
{
  color_convert_table[COLOR_BLACK] = 0x0000;
  color_convert_table[COLOR_WHITE] = 0xFFFF;
  color_convert_table[COLOR_RED] = 0xF800;
  color_convert_table[COLOR_GREEN] = 0x07E0;
  color_convert_table[COLOR_BLUE] = 0x001F;
  color_convert_table[COLOR_YELLOW] = 0xFFE0;
  color_convert_table[COLOR_GRAY] = 0X8430;
}

// Initialize display
void display_init(void)
{
  display_init_conv_table();
  display_init_pins();
  display_delay(100000);
  
  DISP_RST_SET_LOW;
  display_delay(100000);
  DISP_RST_SET_HIGH;
  
  display_delay(100000);
  
  display_write_cmd(0x11);//Sleep exit 
  display_delay(100000);
  display_write_cmd(0x21); 
  display_write_cmd(0x21); 

  display_write_cmd(0xB1); 
  display_write_data8(0x05);
  display_write_data8(0x3A);
  display_write_data8(0x3A);

  display_write_cmd(0xB2);
  display_write_data8(0x05);
  display_write_data8(0x3A);
  display_write_data8(0x3A);

  display_write_cmd(0xB3); 
  display_write_data8(0x05);  
  display_write_data8(0x3A);
  display_write_data8(0x3A);
  display_write_data8(0x05);
  display_write_data8(0x3A);
  display_write_data8(0x3A);

  display_write_cmd(0xB4);
  display_write_data8(0x03);

  display_write_cmd(0xC0);
  display_write_data8(0x62);
  display_write_data8(0x02);
  display_write_data8(0x04);

  display_write_cmd(0xC1);
  display_write_data8(0xC0);

  display_write_cmd(0xC2);
  display_write_data8(0x0D);
  display_write_data8(0x00);

  display_write_cmd(0xC3);
  display_write_data8(0x8D);
  display_write_data8(0x6A);   

  display_write_cmd(0xC4);
  display_write_data8(0x8D); 
  display_write_data8(0xEE); 

  display_write_cmd(0xC5);  /*VCOM*/
  display_write_data8(0x0E);    

  display_write_cmd(0xE0);
  display_write_data8(0x10);
  display_write_data8(0x0E);
  display_write_data8(0x02);
  display_write_data8(0x03);
  display_write_data8(0x0E);
  display_write_data8(0x07);
  display_write_data8(0x02);
  display_write_data8(0x07);
  display_write_data8(0x0A);
  display_write_data8(0x12);
  display_write_data8(0x27);
  display_write_data8(0x37);
  display_write_data8(0x00);
  display_write_data8(0x0D);
  display_write_data8(0x0E);
  display_write_data8(0x10);

  display_write_cmd(0xE1);
  display_write_data8(0x10);
  display_write_data8(0x0E);
  display_write_data8(0x03);
  display_write_data8(0x03);
  display_write_data8(0x0F);
  display_write_data8(0x06);
  display_write_data8(0x02);
  display_write_data8(0x08);
  display_write_data8(0x0A);
  display_write_data8(0x13);
  display_write_data8(0x26);
  display_write_data8(0x36);
  display_write_data8(0x00);
  display_write_data8(0x0D);
  display_write_data8(0x0E);
  display_write_data8(0x10);

  display_write_cmd(0x3A); 
  display_write_data8(0x05);

  display_write_cmd(0x36);
  display_write_data8(0xA8);//

  display_write_cmd(0x29); 

  display_delay(100000);

  display_x = 0;
  display_y = 0;
}

// Clear display
void display_clear(void)
{

}

void LCD_SetCursor(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t  Yend)
{ 
  Xstart = Xstart + 1;
  Xend = Xend + 1;
  Ystart = Ystart + 26;
  Yend = Yend+26;
  
  display_write_cmd(0x2a);
  display_write_data8(Xstart>>8);
  display_write_data8(Xstart);
  display_write_data8(Xend >>8);
  display_write_data8(Xend );
  
  display_write_cmd(0x2b);
  display_write_data8(Ystart>>8);
  display_write_data8(Ystart);
  display_write_data8(Yend>>8);
  display_write_data8(Yend);
  
  display_write_cmd(0x2C);
}

//Send data from framebuffer to LCD
void display_send_full_framebuffer(uint8_t* data)
{
  uint8_t* tmp_ptr = data;
  
  LCD_SetCursor(0, 0, DISP_WIDTH - 1, DISP_HEIGHT - 1);
  for(uint16_t y = 0; y < DISP_HEIGHT; y++)
  {
    DISP_CSN_SET_LOW;
    DISP_DC_SET_HIGH;
    
    for(uint16_t x = 0; x < DISP_WIDTH; x++)
    {
      uint16_t color16 = color_convert_table[*tmp_ptr];
      uint8_t tmp = (color16 >> 8) & 0xff;
      
      while(SPI_I2S_GetFlagStatus(DISPLAY_SPI_NAME, SPI_I2S_FLAG_TXE) == RESET) {}
      SPI_SendData8(DISPLAY_SPI_NAME, tmp);
      while(SPI_I2S_GetFlagStatus(DISPLAY_SPI_NAME, SPI_I2S_FLAG_TXE) == RESET) {}
      SPI_SendData8(DISPLAY_SPI_NAME, (color16 & 0xFF));
      /*
      SPI_SendData8(DISPLAY_SPI_NAME, tmp);
      while(SPI_I2S_GetFlagStatus(DISPLAY_SPI_NAME, SPI_I2S_FLAG_BSY) == SET) {}
      SPI_SendData8(DISPLAY_SPI_NAME, (color16 & 0xFF));
      while(SPI_I2S_GetFlagStatus(DISPLAY_SPI_NAME, SPI_I2S_FLAG_BSY) == SET) {}
      */
      tmp_ptr++;
    }
    while(SPI_I2S_GetFlagStatus(DISPLAY_SPI_NAME, SPI_I2S_FLAG_BSY) == SET) {}
    DISP_CSN_SET_HIGH;
    //tmp_ptr += LCD_WIDTH;
  }
}

//Init SPI for display communication
void display_spi_init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
  
  //Common
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  
  GPIO_InitStructure.GPIO_Pin = DISPLAY_CLK_PIN;
  GPIO_Init(DISPLAY_CLK_GPIO, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = DISPLAY_MOSI_PIN;
  GPIO_Init(DISPLAY_MOSI_GPIO, &GPIO_InitStructure);
  
  GPIO_PinAFConfig(DISPLAY_CLK_GPIO, DISPLAY_CLK_SRC, DISPLAY_CLK_AFIO);
  GPIO_PinAFConfig(DISPLAY_MOSI_GPIO, DISPLAY_MOSI_SRC, DISPLAY_MOSI_AFIO);
  
  SPI_I2S_DeInit(DISPLAY_SPI_NAME);
  SPI_StructInit(&SPI_InitStructure);
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_Init(DISPLAY_SPI_NAME, &SPI_InitStructure);
  
  SPI_Cmd(DISPLAY_SPI_NAME, ENABLE);
}
