#ifndef GPIO_HAL_H
#define GPIO_HAL_H

/** @defgroup hal HAL
 *  @brief Hardware Abstraction Layer.
 *  @{
 *  @defgroup gpio_hal GPIO HAL
 *  @brief GPIO driver for ESP32-S3.
 *  @{
 * 
 * @section genDesc General Description
 *
 * This driver provide functions to configure and handle the ESP32-S3 General
 * Purpose Input-Outputs.
 * 
 * @note GPIO_0, GPIO_3, GPIO_45 and GPIO_46 are strapping pins used to
 * select the boot mode and the internal ROM behaviour. Avoid connecting them
 * to low impedance loads (or loads that pull them up/down) that could change
 * their state during the board reset/power-up sequence.
 * 
 * @note GPIO_43 and GPIO_44 are used by UART0 (USB-to-serial bridge /
 * serial console on most ESP32-S3-DevKitC boards). Avoid using them if the
 * console is needed.
 * 
 * @note GPIO_26 to GPIO_37 are NOT available on modules with integrated SPI
 * flash / PSRAM (e.g. ESP32-S3-WROOM-1): they are connected to the external
 * SPI flash (GPIO_26..GPIO_32) and to the PSRAM (GPIO_33..GPIO_37). Using
 * them will make the device crash.
 * 
 * @note Unlike the classic ESP32, the ESP32-S3 has no input-only pins: every
 * GPIO can be used as both input and output and all of them support internal
 * pull-up / pull-down resistors.
 * 
 * @author Juan Cerrudo
 *
 * @section changelog
 *
 * |   Date	    | Description                                    						|
 * |:----------:|:----------------------------------------------------------------------|
 * | 23/10/2023 | Document creation		                         						|
 * | 18/06/2026 | Update documentation		                         					|
 * | 24/08/2026 | Ported from ESP32-C6 to ESP32		                         			|
 * | 28/08/2026 | Ported from ESP32 to ESP32-S3		                         			|
 * 
 **/

/*==================[inclusions]=============================================*/
#include <stdbool.h>
#include <stdint.h>
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/
/**
 * @brief GPIO direction (input or output).
 * 
 */
typedef enum {
	GPIO_INPUT = 0, 	/**< Input with pull-up resistor */
	GPIO_OUTPUT			/**< Output */
	} io_t;

/**
 * @brief ESP32-S3 available GPIOs (not all of them are available on every
 * board, see the notes above for restricted/unavailable pins).
 * 
 */
typedef enum gpio_list{
	GPIO_0=0, 	/**< GPIO0 - strapping pin */
	GPIO_1, 	/**< GPIO1 */
	GPIO_2, 	/**< GPIO2 */
	GPIO_3, 	/**< GPIO3 - strapping pin, UART0 RX */
	GPIO_4, 	/**< GPIO4 */
	GPIO_5, 	/**< GPIO5 */
	GPIO_6, 	/**< GPIO6 */
	GPIO_7, 	/**< GPIO7 */
	GPIO_8, 	/**< GPIO8 */
	GPIO_9, 	/**< GPIO9 */
	GPIO_10,	/**< GPIO10 */
	GPIO_11,	/**< GPIO11 */
	GPIO_12,	/**< GPIO12 */
	GPIO_13,	/**< GPIO13 */
	GPIO_14,	/**< GPIO14 */
	GPIO_15,	/**< GPIO15 */
	GPIO_16,	/**< GPIO16 */
	GPIO_17,	/**< GPIO17 */
	GPIO_18,	/**< GPIO18 */
	GPIO_19,	/**< GPIO19 */
	GPIO_20,	/**< GPIO20 */
	GPIO_21,	/**< GPIO21 */
	GPIO_22,	/**< GPIO22 */
	GPIO_23,	/**< GPIO23 */
	GPIO_24,	/**< GPIO24 */
	GPIO_25,	/**< GPIO25 */
	GPIO_26,	/**< not available - external SPI flash (CLK) */
	GPIO_27,	/**< not available - external SPI flash */
	GPIO_28,	/**< not available - external SPI flash */
	GPIO_29,	/**< not available - external SPI flash */
	GPIO_30,	/**< not available - external SPI flash */
	GPIO_31,	/**< not available - external SPI flash */
	GPIO_32,	/**< not available - external SPI flash */
	GPIO_33,	/**< not available - PSRAM (if present) */
	GPIO_34,	/**< not available - PSRAM (if present) */
	GPIO_35,	/**< not available - PSRAM (if present) */
	GPIO_36,	/**< not available - PSRAM (if present) */
	GPIO_37,	/**< not available - PSRAM (if present) */
	GPIO_38,	/**< GPIO38 (USB D- on DevKitC, usable as GPIO) */
	GPIO_39,	/**< GPIO39 (USB D+ on DevKitC, usable as GPIO) */
	GPIO_40,	/**< GPIO40 */
	GPIO_41,	/**< GPIO41 */
	GPIO_42,	/**< GPIO42 */
	GPIO_43,	/**< GPIO43 - UART0 TX */
	GPIO_44,	/**< GPIO44 - UART0 RX */
	GPIO_45,	/**< GPIO45 - strapping pin */
	GPIO_46,	/**< GPIO46 - strapping pin */
	GPIO_47,	/**< GPIO47 */
	GPIO_48,	/**< GPIO48 */
} gpio_t;

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/
/**
 * @brief GPIO initialization
 * 
 * @param pin GPIO number
 * @param io GPIO direction
 */
void GPIOInit(gpio_t pin, io_t io);

/**
 * @brief Change GPIO state to high
 * 
 * @param pin GPIO number
 */
void GPIOOn(gpio_t pin);

/**
 * @brief Change GPIO state to low
 * 
 * @param pin GPIO number
 */
void GPIOOff(gpio_t pin);

/**
 * @brief Change GPIO state
 * 
 * @param pin GPIO number
 * @param state GPIO state (true: high - false: low)
 */
void GPIOState(gpio_t pin, bool state);

/**
 * @brief Invert GPIO state
 * 
 * @param pin GPIO number
 */
void GPIOToggle(gpio_t pin);

/**
 * @brief Reads GPIO state
 * 
 * @param pin 
 * @return true GPIO input high
 * @return false GPIO input low
 */
bool GPIORead(gpio_t pin);

/**
 * @brief Configure GPIO input interruption
 * 
 * @param pin GPIO number
 * @param ptr_int_func Pointer to callback function
 * @param edge true: positive edge - false: negative edge
 * @param args 
 */
void GPIOActivInt(gpio_t pin, void *ptr_int_func, bool edge, void *args);

/**
 * @brief GPIO de-initialization
 * 
 */
void GPIODeinit(void);

/** @} */
/** @} */

#endif /* #ifndef GPIO_HAL_H */

/*==================[end of file]============================================*/
