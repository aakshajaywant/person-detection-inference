#include "max32664.h"
#include "app_config.h"
#include <string.h>

/* MAX32664 command bytes based on SparkFun Bio Sensor Hub command model. */
#define MAX32664_ADDR_8BIT           (BIO_I2C_ADDR_7BIT << 1)

#define HUB_STATUS                   0x00
#define SET_DEVICE_MODE              0x01
#define READ_DEVICE_MODE             0x02
#define OUTPUT_MODE                  0x10
#define READ_DATA_OUTPUT             0x12
#define ENABLE_SENSOR                0x44
#define ENABLE_ALGORITHM             0x52

#define SET_FORMAT                   0x00
#define NUM_SAMPLES                  0x00
#define READ_DATA                    0x01
#define ENABLE_MAX30101              0x03
#define ENABLE_AGC_ALGO              0x00
#define ENABLE_WHRM_ALGO             0x02

#define EXIT_BOOTLOADER              0x00
#define SENSOR_AND_ALGORITHM         0x03
#define ALGO_DATA                    0x02
#define ENABLE                       0x01
#define DISABLE                      0x00

#define CMD_DELAY_MS                 45U
#define I2C_TIMEOUT_MS               100U

static I2C_HandleTypeDef *s_i2c = NULL;

static max32664_status_t write_cmd(const uint8_t *tx, uint16_t len, uint32_t delay_ms)
{
    if (s_i2c == NULL || tx == NULL || len == 0) return MAX32664_ERR_BAD_PARAM;

    if (HAL_I2C_Master_Transmit(s_i2c, MAX32664_ADDR_8BIT, (uint8_t *)tx, len, I2C_TIMEOUT_MS) != HAL_OK) {
        return MAX32664_ERR_I2C;
    }
    HAL_Delay(delay_ms);

    uint8_t status = 0xFF;
    if (HAL_I2C_Master_Receive(s_i2c, MAX32664_ADDR_8BIT, &status, 1, I2C_TIMEOUT_MS) != HAL_OK) {
        return MAX32664_ERR_I2C;
    }

    return (status == 0x00) ? MAX32664_OK : MAX32664_ERR_STATUS;
}

static max32664_status_t read_cmd(uint8_t family, uint8_t index, uint8_t *rx, uint16_t rx_len)
{
    if (s_i2c == NULL || rx == NULL || rx_len == 0) return MAX32664_ERR_BAD_PARAM;

    uint8_t tx[2] = { family, index };
    if (HAL_I2C_Master_Transmit(s_i2c, MAX32664_ADDR_8BIT, tx, sizeof(tx), I2C_TIMEOUT_MS) != HAL_OK) {
        return MAX32664_ERR_I2C;
    }

    HAL_Delay(2);

    if (HAL_I2C_Master_Receive(s_i2c, MAX32664_ADDR_8BIT, rx, rx_len, I2C_TIMEOUT_MS) != HAL_OK) {
        return MAX32664_ERR_I2C;
    }

    return (rx[0] == 0x00) ? MAX32664_OK : MAX32664_ERR_STATUS;
}

static max32664_status_t read_cmd3(uint8_t family, uint8_t index, uint8_t write_byte, uint8_t *rx, uint16_t rx_len)
{
    if (s_i2c == NULL || rx == NULL || rx_len == 0) return MAX32664_ERR_BAD_PARAM;

    uint8_t tx[3] = { family, index, write_byte };
    if (HAL_I2C_Master_Transmit(s_i2c, MAX32664_ADDR_8BIT, tx, sizeof(tx), I2C_TIMEOUT_MS) != HAL_OK) {
        return MAX32664_ERR_I2C;
    }

    HAL_Delay(2);

    if (HAL_I2C_Master_Receive(s_i2c, MAX32664_ADDR_8BIT, rx, rx_len, I2C_TIMEOUT_MS) != HAL_OK) {
        return MAX32664_ERR_I2C;
    }

    return (rx[0] == 0x00) ? MAX32664_OK : MAX32664_ERR_STATUS;
}

static void app_mode_reset(void)
{
    /* MAX32664 app mode: MFIO high while reset is asserted, then release reset. */
    HAL_GPIO_WritePin(BIO_MFIO_GPIO_Port, BIO_MFIO_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(BIO_RST_GPIO_Port, BIO_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(BIO_RST_GPIO_Port, BIO_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(100);
}

max32664_status_t max32664_begin(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == NULL) return MAX32664_ERR_BAD_PARAM;
    s_i2c = hi2c;

    app_mode_reset();

    if (HAL_I2C_IsDeviceReady(s_i2c, MAX32664_ADDR_8BIT, 3, I2C_TIMEOUT_MS) != HAL_OK) {
        return MAX32664_ERR_I2C;
    }

    uint8_t rx[2] = {0};
    max32664_status_t st = read_cmd(READ_DEVICE_MODE, 0x00, rx, sizeof(rx));
    if (st != MAX32664_OK) return st;

    return MAX32664_OK;
}

max32664_status_t max32664_config_bpm(void)
{
    max32664_status_t st;

    /* Enable MAX30101 optical sensor. */
    uint8_t enable_sensor[] = { ENABLE_SENSOR, ENABLE_MAX30101, ENABLE };
    st = write_cmd(enable_sensor, sizeof(enable_sensor), CMD_DELAY_MS);
    if (st != MAX32664_OK) return st;

    /* Enable automatic gain control algorithm. */
    uint8_t enable_agc[] = { ENABLE_ALGORITHM, ENABLE_AGC_ALGO, ENABLE };
    st = write_cmd(enable_agc, sizeof(enable_agc), CMD_DELAY_MS);
    if (st != MAX32664_OK) return st;

    /* Enable MaximFast wrist HRM / SpO2 algorithm. */
    uint8_t enable_whrm[] = { ENABLE_ALGORITHM, ENABLE_WHRM_ALGO, ENABLE };
    st = write_cmd(enable_whrm, sizeof(enable_whrm), CMD_DELAY_MS);
    if (st != MAX32664_OK) return st;

    /* Output only algorithm data: HR, confidence, SpO2, status. */
    uint8_t output_mode[] = { OUTPUT_MODE, SET_FORMAT, ALGO_DATA };
    st = write_cmd(output_mode, sizeof(output_mode), CMD_DELAY_MS);
    if (st != MAX32664_OK) return st;

    return MAX32664_OK;
}

max32664_status_t max32664_read_bpm(max32664_sample_t *sample)
{
    if (sample == NULL) return MAX32664_ERR_BAD_PARAM;
    memset(sample, 0, sizeof(*sample));

    uint8_t count_buf[2] = {0};
    max32664_status_t st = read_cmd(READ_DATA_OUTPUT, NUM_SAMPLES, count_buf, sizeof(count_buf));
    if (st != MAX32664_OK) return st;

    uint8_t samples = count_buf[1];
    if (samples == 0) return MAX32664_ERR_TIMEOUT;

    /* Response: status byte + algorithm payload.
     * Algorithm payload is commonly:
     * HR MSB, HR LSB, confidence, SpO2 MSB, SpO2 LSB, status.
     */
    uint8_t data[7] = {0};
    st = read_cmd(READ_DATA_OUTPUT, READ_DATA, data, sizeof(data));
    if (st != MAX32664_OK) return st;

    sample->heart_rate_x10 = ((uint16_t)data[1] << 8) | data[2];
    sample->confidence = data[3];
    sample->spo2 = ((uint16_t)data[4] << 8) | data[5];
    sample->status = data[6];

    return MAX32664_OK;
}

bool max32664_finger_detected(const max32664_sample_t *sample)
{
    return sample != NULL && sample->status == 3U;
}

const char *max32664_status_string(max32664_status_t status)
{
    switch (status) {
        case MAX32664_OK: return "OK";
        case MAX32664_ERR_I2C: return "I2C_ERROR";
        case MAX32664_ERR_STATUS: return "SENSOR_STATUS_ERROR";
        case MAX32664_ERR_TIMEOUT: return "NO_FIFO_SAMPLE";
        case MAX32664_ERR_BAD_PARAM: return "BAD_PARAM";
        default: return "UNKNOWN";
    }
}
