#ifndef __SLOW_SCOPE_H
#define __SLOW_SCOPE_H

#include "mode_controlling.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  ADC_SLOW_IDLE = 0,
  ADC_SLOW_CAPTURE_RUNNING,
  ADC_SLOW_PROCESSING_DATA,
  ADC_SLOW_PROCESSING_DATA_DONE
} slow_scope_data_processing_state_t;

typedef struct
{
  float max_voltage;
  float grid_step;
} grid_mode_item_t;

void slow_scope_processing_main_mode_changed(void);
void slow_scope_processing_handler(void);

void slow_scope_draw_menu(menu_draw_type_t draw_type);
void slow_scope_upper_button_pressed(void);

#endif 

