#ifndef GPIO_HAL_H
#define GPIO_HAL_H

/** @defgroup hal HAL
 *  @brief Hardware Abstraction Layer.
 *  @{
 *  @defgroup gpio_hal GPIO HAL
 *  @brief GPIO driver for ESP32.
 *  @{
 * 
 * @section genDesc General Description
 *
 * This driver provide functions to configure and handle the ESP32 General
 * Purpose Input-Outputs.
 * 
 * @note GPIO_0, GPIO_2, GPIO_5, GPIO_12 and GPIO_15 are strapping pins used to
 * select the boot mode and the SPI flash voltage. Avoid connecting them to
 * low impedance loads (or loads that pull them up/down) that could change
 * their state during the board reset/power-up sequence.
 * 
 * @note GPIO_1 and GPIO_3 are used by UART0 (serial console/programming), avoid
 * using them if the console is needed.
 * 
 * @note GPIO_6 to GPIO_11 are NOT available, they are connected to the
 * integrated SPI flash memory. Using them will make the device crash.
 * 
 * @note GPIO_16 and GPIO_17 are used for PSRAM on WROVER modules, avoid using
 * them on boards with PSRAM.
 * 
 * @note GPIO_20, GPIO_24, GPIO_28, GPIO_29, GPIO_30 and GPIO_31 are NOT
 * available (not brought out of the chip package).
 * 
 * @note GPIO_34 to GPIO_39 are input only pins, they do not have internal
 * pull-up/pull-down resistors and cannot be used as outputs.
 * 
 * @author Albano Peñalva
 *
 * @section changelog
 *
 * |   Date	    | Description                                    						|
 * |:----------:|:----------------------------------------------------------------------|
 * | 23/10/2023 | Document creation		                         						|
 * | 18/06/2026 | Update documentation		                         					|
 * | 24/08/2026 | Ported from ESP32-C6 to ESP32		                         			|
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
 * @brief ESP32 available GPIOs (not all of them are available on every board,
 * see the notes above for restricted/unavailable pins)
 * 
 */
typedef enum gpio_list{
	GPIO_0=0, 	/**< GPIO0 - strapping pin */
	GPIO_1, 	/**< GPIO1 - UART0 TX */
	GPIO_2, 	/**< GPIO2 - strapping pin */
	GPIO_3, 	/**< GPIO3 - UART0 RX */
	GPIO_4, 	/**< GPIO4 - TOUCH0 */
	GPIO_5, 	/**< GPIO5 - strapping pin */
	GPIO_6, 	/**< not available - integrated SPI flash (CLK) */
	GPIO_7, 	/**< not available - integrated SPI flash (SD0) */
	GPIO_8, 	/**< not available - integrated SPI flash (SD1) */
	GPIO_9, 	/**< not available - integrated SPI flash (SD2) */
	GPIO_10, 	/**< not available - integrated SPI flash (SD3) */
	GPIO_11, 	/**< not available - integrated SPI flash (CMD) */
	GPIO_12, 	/**< GPIO12 - strapping pin, TOUCH5 */
	GPIO_13, 	/**< GPIO13 - TOUCH4 */
	GPIO_14, 	/**< GPIO14 - TOUCH6 */
	GPIO_15, 	/**< GPIO15 - strapping pin, TOUCH3 */
	GPIO_16, 	/**< GPIO16 - PSRAM on WROVER modules */
	GPIO_17, 	/**< GPIO17 - PSRAM on WROVER modules */
	GPIO_18, 	/**< GPIO18 */
	GPIO_19, 	/**< GPIO19 */
	GPIO_20, 	/**< not available */
	GPIO_21,	/**< GPIO21 */
	GPIO_22, 	/**< GPIO22 */
	GPIO_23, 	/**< GPIO23 */
	GPIO_24, 	/**< not available */
	GPIO_25, 	/**< GPIO25 - DAC1 */
	GPIO_26, 	/**< GPIO26 - DAC2 */
	GPIO_27, 	/**< GPIO27 - TOUCH7 */
	GPIO_28, 	/**< not available */
	GPIO_29, 	/**< not available */
	GPIO_30, 	/**< not available */
	GPIO_31, 	/**< not available */
	GPIO_32, 	/**< GPIO32 - TOUCH9 */
	GPIO_33, 	/**< GPIO33 - TOUCH8 */
	GPIO_34, 	/**< GPIO34 - input only */
	GPIO_35, 	/**< GPIO35 - input only */
	GPIO_36, 	/**< GPIO36 - input only */
	GPIO_37, 	/**< GPIO37 - input only, usually not broken out */
	GPIO_38, 	/**< GPIO38 - input only, usually not broken out */
	GPIO_39, 	/**< GPIO39 - input only */
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
