#ifndef __FREQ_MEASUREMENT_H
#define __FREQ_MEASUREMENT_H


/* Exported types ------------------------------------------------------------*/
typedef enum
{
  FREQ_MEASUREMENT_IDLE = 0,
  FREQ_MEASUREMENT_CAPTURE_RUNNING,
  FREQ_MEASUREMENT_PROCESSING_DATA,
  FREQ_MEASUREMENT_PROCESSING_DATA_DONE
} freq_measurement_state_t;

// Default voltage for comparator
#define FREQ_TRIGGER_DEFAULT_V         (1.5f)

/* Exported functions ------------------------------------------------------- */
extern freq_measurement_state_t freq_measurement_state;
extern float freq_comparator_threshold_v;
extern uint32_t freq_measurement_calc_frequency;

void freq_measurement_start_freq_capture(void);
void freq_measurement_processing_handler(void);
void freq_measurement_main_mode_changed(void);
void freq_measurement_init_timers(void);

#endif 

