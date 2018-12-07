/* Includes ------------------------------------------------------------------*/
#include "keys_controlling.h"
#include "menu_controlling.h"
#include "stm32f30x_gpio.h"
#include "string.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
//Time in ms
#define KEY_HOLD_TIME            900

//Time in ms
#define KEY_PRESSED_TIME         50

//Time in ms
#define KEY_RELEASE_TIME         50

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
key_item_t key_down;
key_item_t key_up;

extern volatile uint32_t ms_tick;

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

void keys_init(void)
{
  key_down.gpio_name = BUTTON1_GPIO;
  key_down.pin_name = BUTTON1_PIN;
  keys_functons_init_hardware(&key_down);
  
  key_up.gpio_name = BUTTON2_GPIO;
  key_up.pin_name = BUTTON2_PIN;
  keys_functons_init_hardware(&key_up);
}

void key_handling(void)
{
  keys_functons_update_key_state(&key_down);
  keys_functons_update_key_state(&key_up);
  
  if ((key_down.prev_state == KEY_PRESSED) && 
      (key_down.state == KEY_WAIT_FOR_RELEASE))
  {
    menu_lower_button_pressed();
  }
}

//*****************************************************************************

// Initialize single key pin
void keys_functons_init_hardware(key_item_t* key_item)
{
  if (key_item == NULL)
    return;
  
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Pin = key_item->pin_name;
  GPIO_Init(key_item->gpio_name, &GPIO_InitStructure);
  
  key_item->state = KEY_RELEASED;
}

void keys_functons_update_key_state(key_item_t* key_item)
{
  key_item->prev_state = key_item->state;
  
  if ((key_item->gpio_name->IDR & key_item->pin_name) != 0)
    key_item->current_state = 1;
  else
    key_item->current_state = 0;
  
  if ((key_item->state == KEY_RELEASED) && (key_item->current_state != 0))
  {
    //key presed now
    key_item->state = KEY_PRESSED_WAIT;
    key_item->key_timestamp = ms_tick;
    return;
  }
  
  if (key_item->state == KEY_PRESSED_WAIT)
  {
    uint32_t delta_time = ms_tick - key_item->key_timestamp;
    if (delta_time > KEY_PRESSED_TIME)
    {
      if (key_item->current_state != 0)
        key_item->state = KEY_PRESSED;
      else
        key_item->state = KEY_RELEASED;
    }
    return;
  }
  
  if ((key_item->state == KEY_PRESSED) || (key_item->state == KEY_HOLD))
  {
    // key not pressed
    if (key_item->current_state == 0)
    {
      key_item->state = KEY_WAIT_FOR_RELEASE;// key is locked here
      key_item->key_timestamp = ms_tick;
      return;
    }
  }
  
  if (key_item->state == KEY_WAIT_FOR_RELEASE)
  {
    uint32_t delta_time = ms_tick - key_item->key_timestamp;
    if (delta_time > KEY_RELEASE_TIME)
    {
      key_item->state = KEY_RELEASED;
      return;
    }
  }
  
  if ((key_item->state == KEY_PRESSED) && (key_item->current_state != 0))
  {
    //key still presed now
    uint32_t delta_time = ms_tick - key_item->key_timestamp;
    if (delta_time > KEY_HOLD_TIME)
    {
      key_item->state = KEY_HOLD;
      return;
    }
  }
}
