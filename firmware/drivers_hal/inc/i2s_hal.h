#ifndef I2S_HAL_H
#define I2S_HAL_H

/** @defgroup hal HAL
 *  @brief Hardware Abstraction Layer.
 *  @{
 *  @defgroup i2s_hal I2S HAL
 *  @brief I2S driver for ESP32.
 *  @{
 *
 * @section genDesc General Description
 *
 * Thin wrapper over ESP-IDF 6.x channel-based I2S driver (driver/i2s_std.h).
 * Supports standard mode (Philips/MSB/PCM) with TX and RX directions.
 *
 * @author Audio Relay Project
 *
 **/

/*==================[inclusions]=============================================*/
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/**
 * @brief I2S HAL configuration.
 */
typedef struct {
    int bclk_pin;          /*!< Bit clock GPIO pin */
    int ws_pin;            /*!< Word select (LRCLK) GPIO pin */
    int data_pin;          /*!< Data GPIO pin (MOSI for TX, MISO for RX) */
    uint32_t sample_rate;  /*!< Sample rate in Hz (e.g. 44100) */
    int bits_per_sample;   /*!< Bits per sample: 16 or 32 */
    bool is_stereo;        /*!< true: stereo (2 slots), false: mono (1 slot) */
    bool is_master;        /*!< true: drives BCLK/WS, false: receives their clock */
} i2s_hal_config_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @brief Initialize I2S in TX mode (master).
 *
 * @param config  I2S configuration (pins, sample rate, etc.)
 * @return true if successful.
 */
bool I2sHalInitTx(const i2s_hal_config_t *config);

/**
 * @brief Initialize I2S in RX mode (master).
 *
 * @param config  I2S configuration (pins, sample rate, etc.)
 * @return true if successful.
 */
bool I2sHalInitRx(const i2s_hal_config_t *config);

/**
 * @brief Write data to I2S TX channel.
 *
 * @param buf     Data buffer.
 * @param len     Number of bytes to write.
 * @param timeout_ms  Timeout in milliseconds.
 * @return Number of bytes actually written, or -1 on error.
 */
int I2sHalWrite(const uint8_t *buf, size_t len, uint32_t timeout_ms);

/**
 * @brief Read data from I2S RX channel.
 *
 * @param buf     Output buffer.
 * @param len     Maximum bytes to read.
 * @param timeout_ms  Timeout in milliseconds.
 * @return Number of bytes actually read, or -1 on error.
 */
int I2sHalRead(uint8_t *buf, size_t len, uint32_t timeout_ms);

/**
 * @brief Deinitialize I2S and free resources.
 */
void I2sHalDeinit(void);

/** @} */
/** @} */

#endif /* #ifndef I2S_HAL_H */

/*==================[end of file]============================================*/
