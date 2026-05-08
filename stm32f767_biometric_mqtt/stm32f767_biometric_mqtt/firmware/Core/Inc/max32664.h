#ifndef MAX32664_H
#define MAX32664_H

#include "stm32f7xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t ir_led;
    uint32_t red_led;
    uint16_t heart_rate_x10;  /* 0.1 bpm units */
    uint8_t confidence;       /* 0-100 */
    uint16_t spo2;            /* percent */
    uint8_t status;           /* 0 success, 1 not ready, 2 object, 3 finger */
} max32664_sample_t;

typedef enum {
    MAX32664_OK = 0,
    MAX32664_ERR_I2C,
    MAX32664_ERR_STATUS,
    MAX32664_ERR_TIMEOUT,
    MAX32664_ERR_BAD_PARAM
} max32664_status_t;

max32664_status_t max32664_begin(I2C_HandleTypeDef *hi2c);
max32664_status_t max32664_config_bpm(void);
max32664_status_t max32664_read_bpm(max32664_sample_t *sample);
const char *max32664_status_string(max32664_status_t status);
bool max32664_finger_detected(const max32664_sample_t *sample);

#endif /* MAX32664_H */
