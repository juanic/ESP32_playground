/**
 * @file gpio_mcu.c
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 * @brief 
 * @version 0.1
 * @date 2023-10-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

/*==================[inclusions]=============================================*/
#include "gpio_hal.h"
#include <stdint.h>
#include "driver/gpio.h"
/*==================[macros and definitions]=================================*/
#define GPIO_QTY 	40
typedef struct{
	uint64_t pin;				/*!< GPIO pin */
	gpio_mode_t mode;			/*!< Input/Output mode */
	gpio_pull_mode_t pull;		/*!< GPIO pull-up/pull-down resistor */
	bool state;					/*!< GPIO output state */
} digital_io_t;
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/
static bool GPIOIsValid(gpio_t pin);
/*==================[internal data definition]===============================*/
/* GPIO6 to GPIO11 (integrated SPI flash) and GPIO20/24/28/29/30/31 (not brought
 * out of the chip package) are placeholder entries, GPIOInit() rejects them
 * before they are ever used. */
digital_io_t gpio_list[GPIO_QTY] = {
	{GPIO_NUM_0, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* Configuration GPIO0*/
	{GPIO_NUM_1, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* Configuration GPIO1*/
	{GPIO_NUM_2, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* Configuration GPIO2*/
	{GPIO_NUM_3, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* Configuration GPIO3*/
	{GPIO_NUM_4, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* Configuration GPIO4*/
	{GPIO_NUM_5, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* Configuration GPIO5*/
	{GPIO_NUM_0, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* GPIO6 - not available */
	{GPIO_NUM_0, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* GPIO7 - not available */
	{GPIO_NUM_0, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* GPIO8 - not available */
	{GPIO_NUM_0, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* GPIO9 - not available */
	{GPIO_NUM_0, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* GPIO10 - not available */
	{GPIO_NUM_0, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* GPIO11 - not available */
	{GPIO_NUM_12, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* Configuration GPIO12*/
	{GPIO_NUM_13, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* Configuration GPIO13*/
	{GPIO_NUM_14, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* Configuration GPIO14*/
	{GPIO_NUM_15, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* Configuration GPIO15*/
	{GPIO_NUM_16, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* Configuration GPIO16*/
	{GPIO_NUM_17, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* Configuration GPIO17*/
	{GPIO_NUM_18, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* Configuration GPIO18*/
	{GPIO_NUM_19, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* Configuration GPIO19*/
	{GPIO_NUM_0, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* GPIO20 - not available */
	{GPIO_NUM_21, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* Configuration GPIO21*/
	{GPIO_NUM_22, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* Configuration GPIO22*/
	{GPIO_NUM_23, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* Configuration GPIO23*/
	{GPIO_NUM_0, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* GPIO24 - not available */
	{GPIO_NUM_25, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* Configuration GPIO25*/
	{GPIO_NUM_26, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* Configuration GPIO26*/
	{GPIO_NUM_27, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* Configuration GPIO27*/
	{GPIO_NUM_0, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* GPIO28 - not available */
	{GPIO_NUM_0, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* GPIO29 - not available */
	{GPIO_NUM_0, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* GPIO30 - not available */
	{GPIO_NUM_0, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* GPIO31 - not available */
	{GPIO_NUM_32, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* Configuration GPIO32*/
	{GPIO_NUM_33, GPIO_MODE_DISABLE, GPIO_PULLUP_ONLY, false}, /* Configuration GPIO33*/
	{GPIO_NUM_34, GPIO_MODE_DISABLE, GPIO_FLOATING, false}, /* Configuration GPIO34, input only, no pull resistor*/
	{GPIO_NUM_35, GPIO_MODE_DISABLE, GPIO_FLOATING, false}, /* Configuration GPIO35, input only, no pull resistor*/
	{GPIO_NUM_36, GPIO_MODE_DISABLE, GPIO_FLOATING, false}, /* Configuration GPIO36, input only, no pull resistor*/
	{GPIO_NUM_37, GPIO_MODE_DISABLE, GPIO_FLOATING, false}, /* Configuration GPIO37, input only, no pull resistor*/
	{GPIO_NUM_38, GPIO_MODE_DISABLE, GPIO_FLOATING, false}, /* Configuration GPIO38, input only, no pull resistor*/
	{GPIO_NUM_39, GPIO_MODE_DISABLE, GPIO_FLOATING, false}, /* Configuration GPIO39, input only, no pull resistor*/
};
/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
/** \brief Checks whether a pin is a real, usable GPIO on the ESP32 */
static bool GPIOIsValid(gpio_t pin){
	if(pin > GPIO_39){
		return false;
	}
	switch(pin){
		case GPIO_6:
		case GPIO_7:
		case GPIO_8:
		case GPIO_9:
		case GPIO_10:
		case GPIO_11:
		case GPIO_20:
		case GPIO_24:
		case GPIO_28:
		case GPIO_29:
		case GPIO_30:
		case GPIO_31:
			return false;
		default:
			return true;
	}
}

/*==================[external functions definition]==========================*/
void GPIOInit(gpio_t pin, io_t io){
	if(!GPIOIsValid(pin)){
		return;
	}
	/* GPIO34 to GPIO39 are input only pins, they have no output driver */
	if((pin >= GPIO_34) && (io == GPIO_OUTPUT)){
		return;
	}
	if(io == GPIO_INPUT){
		gpio_list[pin].mode = GPIO_MODE_INPUT;
	} else if(io == GPIO_OUTPUT){
		gpio_list[pin].mode = GPIO_MODE_OUTPUT;
	}
	gpio_reset_pin(gpio_list[pin].pin);
	gpio_set_direction(gpio_list[pin].pin, gpio_list[pin].mode);
	gpio_set_pull_mode(gpio_list[pin].pin, gpio_list[pin].pull);
}

void GPIOOn(gpio_t pin){
	gpio_list[pin].state = true;
	gpio_set_level(gpio_list[pin].pin, gpio_list[pin].state);
}

void GPIOOff(gpio_t pin){
	gpio_list[pin].state = false;
	gpio_set_level(gpio_list[pin].pin, gpio_list[pin].state);
}

void GPIOState(gpio_t pin, bool state){
	gpio_list[pin].state = state;
	gpio_set_level(gpio_list[pin].pin, gpio_list[pin].state);
}

void GPIOToggle(gpio_t pin){
	gpio_list[pin].state = !gpio_list[pin].state;
	gpio_set_level(gpio_list[pin].pin, gpio_list[pin].state);
}

bool GPIORead(gpio_t pin){
	return gpio_get_level(gpio_list[pin].pin);
}

void GPIOActivInt(gpio_t pin, void *ptr_int_func, bool edge, void *args){
	static bool isr_service_installed = false;
	if(edge){
		gpio_set_intr_type(gpio_list[pin].pin, GPIO_INTR_POSEDGE);
	} else{
		gpio_set_intr_type(gpio_list[pin].pin, GPIO_INTR_NEGEDGE);
	}
	if(!isr_service_installed){	
		gpio_install_isr_service(0);
		isr_service_installed = true;
	}
    gpio_isr_handler_add(gpio_list[pin].pin, ptr_int_func, (void *)args);	
}

void GPIODeinit(void){
	
}

/*==================[end of file]============================================*/
