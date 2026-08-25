/**
 * @file touch_hal.c
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 * @brief Touch pad driver for ESP32.
 * @version 0.1
 * @date 2026-08-24
 *
 * @copyright Copyright (c) 2026
 *
 */

/*==================[inclusions]=============================================*/
#include "touch_hal.h"
#include <string.h>
#include "driver/touch_sens.h"
/*==================[macros and definitions]=================================*/
#define TOUCH_PAD_QTY 	10
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/
static bool TouchPadIsValid(touch_t pad);
/*==================[internal data definition]===============================*/
static touch_sensor_handle_t s_touch_handle = NULL;
static touch_channel_handle_t s_touch_chan[TOUCH_PAD_QTY] = {NULL};
static uint32_t s_touch_thresh[TOUCH_PAD_QTY] = {0};
static touch_sensor_sample_config_t s_sample_cfg;
static bool s_scanning = false;
/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
static bool TouchPadIsValid(touch_t pad){
	return (pad < TOUCH_PAD_QTY);
}

/*==================[external functions definition]==========================*/
bool TouchHalInit(void){
	if(s_touch_handle != NULL){
		return true;
	}

	/* Default sampling: 5ms charge duration, 0.5V to 1.7V charge/discharge range */
	s_sample_cfg = (touch_sensor_sample_config_t)TOUCH_SENSOR_V1_DEFAULT_SAMPLE_CONFIG(
		5.0, TOUCH_VOLT_LIM_L_0V5, TOUCH_VOLT_LIM_H_1V7);
	touch_sensor_config_t sens_cfg = TOUCH_SENSOR_DEFAULT_BASIC_CONFIG(1, &s_sample_cfg);

	if(touch_sensor_new_controller(&sens_cfg, &s_touch_handle) != ESP_OK){
		return false;
	}

	touch_sensor_filter_config_t filter_cfg = TOUCH_SENSOR_DEFAULT_FILTER_CONFIG();
	if(touch_sensor_config_filter(s_touch_handle, &filter_cfg) != ESP_OK){
		return false;
	}

	return true;
}

bool TouchHalChannelConfig(touch_t pad, uint32_t threshold){
	if((s_touch_handle == NULL) || !TouchPadIsValid(pad)){
		return false;
	}

	touch_channel_config_t chan_cfg = {
		.abs_active_thresh = {threshold},
		.charge_speed = TOUCH_CHARGE_SPEED_7,
		.init_charge_volt = TOUCH_INIT_CHARGE_VOLT_DEFAULT,
		.group = TOUCH_CHAN_TRIG_GROUP_BOTH,
	};

	if(touch_sensor_new_channel(s_touch_handle, pad, &chan_cfg, &s_touch_chan[pad]) != ESP_OK){
		return false;
	}
	s_touch_thresh[pad] = threshold;
	return true;
}

bool TouchHalStart(void){
	if(s_touch_handle == NULL){
		return false;
	}
	if(touch_sensor_enable(s_touch_handle) != ESP_OK){
		return false;
	}
	if(touch_sensor_start_continuous_scanning(s_touch_handle) != ESP_OK){
		touch_sensor_disable(s_touch_handle);
		return false;
	}
	s_scanning = true;
	return true;
}

bool TouchHalRead(touch_t pad, uint32_t *value){
	if(!TouchPadIsValid(pad) || (s_touch_chan[pad] == NULL) || (value == NULL)){
		return false;
	}
	uint32_t data = 0;
	if(touch_channel_read_data(s_touch_chan[pad], TOUCH_CHAN_DATA_TYPE_SMOOTH, &data) != ESP_OK){
		return false;
	}
	*value = data;
	return true;
}

bool TouchHalIsTouched(touch_t pad){
	uint32_t value = 0;
	if(!TouchHalRead(pad, &value)){
		return false;
	}
	return (value < s_touch_thresh[pad]);
}

void TouchHalDeinit(void){
	if(s_touch_handle == NULL){
		return;
	}
	if(s_scanning){
		touch_sensor_stop_continuous_scanning(s_touch_handle);
		touch_sensor_disable(s_touch_handle);
		s_scanning = false;
	}
	for(int pad = 0; pad < TOUCH_PAD_QTY; pad++){
		if(s_touch_chan[pad] != NULL){
			touch_sensor_del_channel(s_touch_chan[pad]);
			s_touch_chan[pad] = NULL;
		}
	}
	touch_sensor_del_controller(s_touch_handle);
	s_touch_handle = NULL;
}

/*==================[end of file]============================================*/
