#include "SSD1315.h"

uint8_t display_x = 0;
uint8_t display_y = 0;

void display_spi_init(void);

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

// Send data to te display
void display_write_data(unsigned char dat) 
{
  DISP_CSN_SET_LOW;
  DISP_DC_SET_HIGH;
  display_delay(2);
  
  /*
  for (uint8_t i = 0; i < 8; i++) 
  {
    GPIO_ResetBits(DISPLAY_CLK_GPIO, DISPLAY_CLK_PIN);
    display_delay(2);
    if (dat & 0x80)
      GPIO_SetBits(DISPLAY_MOSI_GPIO, DISPLAY_MOSI_PIN);
    else
      GPIO_ResetBits(DISPLAY_MOSI_GPIO, DISPLAY_MOSI_PIN);
    display_delay(2);
    GPIO_SetBits(DISPLAY_CLK_GPIO, DISPLAY_CLK_PIN);
    display_delay(2);
    dat = dat << 1;
  }
  */
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
  
  /*
  for (uint8_t i = 0; i < 8; i++) 
  {
    GPIO_ResetBits(DISPLAY_CLK_GPIO, DISPLAY_CLK_PIN);
    display_delay(2);
    if (cmd & 0x80)
      GPIO_SetBits(DISPLAY_MOSI_GPIO, DISPLAY_MOSI_PIN);
    else
      GPIO_ResetBits(DISPLAY_MOSI_GPIO, DISPLAY_MOSI_PIN);
    display_delay(2);
    GPIO_SetBits(DISPLAY_CLK_GPIO, DISPLAY_CLK_PIN);
    display_delay(2);
    cmd = cmd << 1;
  }
  */
  SPI_SendData8(DISPLAY_SPI_NAME, cmd);
  while(SPI_I2S_GetFlagStatus(DISPLAY_SPI_NAME, SPI_I2S_FLAG_BSY) == SET) {}
  
  display_delay(2);
  DISP_CSN_SET_HIGH;
  display_delay(2);
}


//Specify DDRAM line for COM0 0~63
void display_set_start_line(unsigned char line) 
{
  line |= SSD1315_SET_START_LINE_CMD;
  display_write_cmd(line);
}

// Set page ("string" or "Y")
// "page" - page address 0~7
void display_set_page_address(unsigned char page)
{
  page = SSD1315_SET_PAGE_ADDR | page;
  display_write_cmd(page);
}

// Set column 0-127
// "column" - column address ("X")
void display_set_column_address(unsigned char column) 
{
  display_write_cmd((0x10 | (column >> 4)));
  display_write_cmd((0x0f & column));
}


// Set contrast value
// "value" 0-255
void display_set_contrast_value(unsigned char value) 
{
  display_write_cmd(SSD1315_SET_CONTRAST_CMD);
  display_write_cmd(value);
}

// Initialize display
void display_init(void)
{
  display_init_pins();
  display_delay(100000);
  
  DISP_RST_SET_LOW;
  display_delay(100000);
  DISP_RST_SET_HIGH;
  
  display_delay(100000);
  
  display_write_cmd(0xAE); //  Display off
  display_set_column_address(0);
  display_set_start_line(0);
  display_set_page_address(0);
  display_set_contrast_value(DISP_DEFAULT_CONTRAST);
 
  // This command changes the mapping between 
  // the display data column address and the segment driver.
  display_write_cmd(0xA1); /*set segment remap*/
  display_write_cmd(0xA4);
  
  display_write_cmd(0xA8); /*multiplex ratio*/
  display_write_cmd(0x3F); /*duty = 1/64*/
  
  display_write_cmd(0xC8); /*Com scan direction*/
  
  display_write_cmd(0xD3); /*set display offset*/
  display_write_cmd(0x00);
  
  display_write_cmd(0xD5); /*set osc division*/
  display_write_cmd(0x90);
  
  display_write_cmd(0xD9); /*set pre-charge period*/
  display_write_cmd(0x22);
  
  display_write_cmd(0xDA); /*set COM pins*/
  display_write_cmd(0x12);
  
  display_write_cmd(0xdb); /*set vcomh*/
  display_write_cmd(0x30);
  
  display_write_cmd(0x8d); /*set charge pump enable*/
  display_write_cmd(0x14);
  
    display_write_cmd(SSD1315_NORMAL_MODE_CMD);
  // display_write_cmd(SSD1315_INVERSE_MODE_CMD);
  display_write_cmd(SSD1315_ALL_OFF_CMD);
  // display_write_cmd(SSD1315_ALL_ON_CMD);
 
#if (INVERT_MODE==1)

#else

#endif

  display_write_cmd(0xAF); //  Display on - ok
  
  display_delay(100000);

  display_x = 0;
  display_y = 0;
}

// Clear display
void display_clear(void)
{
  unsigned char x, y;
  
  for (y = 0; y < DISP_PAGE_CNT; y++) //clear page 0~7
  {
    display_write_cmd(SSD1315_SET_PAGE_ADDR + y); // Set page
    display_write_cmd(0x00); // Set column
    display_write_cmd(0x10); // Set column
    for (x = 0; x < DISP_WIDTH; x++) //clear all columns
    {
      display_write_data(0x00);
    }
  }
}

// Set cursor position
void display_gotoxy(unsigned char x, unsigned char y) 
{
  display_x = x;
  display_y = y;
#if (INVERT_MODE==1)
  x++;
#endif
  x = x * 6+2+10;
  display_write_cmd((SSD1315_SET_PAGE_ADDR | (y & 0x0F)));//0xB0 - Page address (page is 8 pixel height)
  display_write_cmd(0x10 | (x >> 4));//0x10 - Most significant column address
  display_write_cmd(0x00 | (x & 0x0f));//0x00 - Least significant column address
}

//Send data from framebuffer to LCD
void display_send_full_framebuffer(uint8_t* data)
{
  unsigned char i, j;
  uint16_t data_counter = 0;
  
  for (i = 0; i < DISP_PAGE_CNT; i++) //page 0~7
  {
    display_write_cmd(SSD1315_SET_PAGE_ADDR + i); //set page
    display_write_cmd(0x00); //set column
    display_write_cmd(0x10); //set column
    for (j = 0; j < DISP_WIDTH; j++)
    {
      display_write_data(data[data_counter++]);
    }
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
