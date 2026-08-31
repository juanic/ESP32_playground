/**
 * @file pwm_hal.c
 * @author Juan Ignacio
 * @brief PWM HAL driver implementation using ESP-IDF LEDC peripheral.
 * @version 0.1
 * @date 2026-08-26
 *
 * @copyright Copyright (c) 2026
 *
 */

/*==================[inclusions]=============================================*/
#include "pwm_hal.h"
#include "driver/ledc.h"
/*==================[macros and definitions]=================================*/
#define PWM_MAX_CHANNELS    4           /*!< Maximum simultaneous PWM outputs */
#define PWM_DUTY_RESOLUTION LEDC_TIMER_13_BIT  /*!< 13-bit → 0..8191 */
#define PWM_DUTY_MAX        ((1 << 13) - 1)    /*!< 8191 */
#define PWM_SPEED_MODE      LEDC_LOW_SPEED_MODE

/** Internal bookkeeping for each allocated PWM channel */
typedef struct {
    bool     in_use;        /*!< true if this slot is allocated */
    gpio_t   gpio;          /*!< HAL GPIO enum value */
    uint8_t  duty_percent;  /*!< Last set duty 0–100 */
    ledc_channel_t channel; /*!< LEDC channel number */
} pwm_slot_t;
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/
static int PwmFindSlot(gpio_t gpio);
static int PwmAllocSlot(gpio_t gpio);

/*==================[internal data definition]===============================*/
static pwm_slot_t pwm_slots[PWM_MAX_CHANNELS] = {0};
static bool timer_initialized = false;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/** Find the slot index for a GPIO that is already initialised.
 *  Returns -1 if not found. */
static int PwmFindSlot(gpio_t gpio){
    for(int i = 0; i < PWM_MAX_CHANNELS; i++){
        if(pwm_slots[i].in_use && pwm_slots[i].gpio == gpio){
            return i;
        }
    }
    return -1;
}

/** Allocate a free slot for a new GPIO.
 *  Returns the slot index or -1 if all slots are taken. */
static int PwmAllocSlot(gpio_t gpio){
    /* Check if already allocated */
    int existing = PwmFindSlot(gpio);
    if(existing >= 0){
        return existing;
    }
    /* Find a free slot */
    for(int i = 0; i < PWM_MAX_CHANNELS; i++){
        if(!pwm_slots[i].in_use){
            return i;
        }
    }
    return -1;
}

/*==================[external functions definition]==========================*/

uint8_t PwmHalInit(gpio_t gpio, uint32_t freq_hz){
    /* Configure the LEDC timer once */
    if(!timer_initialized){
        ledc_timer_config_t timer_cfg = {
            .speed_mode      = PWM_SPEED_MODE,
            .duty_resolution = PWM_DUTY_RESOLUTION,
            .timer_num       = LEDC_TIMER_0,
            .freq_hz         = freq_hz,
            .clk_cfg         = LEDC_AUTO_CLK,
        };
        if(ledc_timer_config(&timer_cfg) != ESP_OK){
            return false;
        }
        timer_initialized = true;
    }

    /* Allocate a channel slot */
    int slot = PwmAllocSlot(gpio);
    if(slot < 0){
        return false;
    }

    /* Configure the LEDC channel */
    ledc_channel_config_t ch_cfg = {
        .gpio_num   = (int)gpio,
        .speed_mode = PWM_SPEED_MODE,
        .channel    = (ledc_channel_t)slot,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 0,
        .hpoint     = 0,
    };
    if(ledc_channel_config(&ch_cfg) != ESP_OK){
        return false;
    }

    /* Register the slot */
    pwm_slots[slot].in_use       = true;
    pwm_slots[slot].gpio         = gpio;
    pwm_slots[slot].duty_percent = 0;
    pwm_slots[slot].channel      = (ledc_channel_t)slot;

    return true;
}

uint8_t PwmHalSetDuty(gpio_t gpio, uint8_t duty_percent){
    int slot = PwmFindSlot(gpio);
    if(slot < 0){
        return false;
    }

    if(duty_percent > 100){
        duty_percent = 100;
    }

    /* Convert 0-100 % to 0-8191 */
    uint32_t duty_raw = (uint32_t)duty_percent * PWM_DUTY_MAX / 100;

    ledc_set_duty(PWM_SPEED_MODE, pwm_slots[slot].channel, duty_raw);
    ledc_update_duty(PWM_SPEED_MODE, pwm_slots[slot].channel);

    pwm_slots[slot].duty_percent = duty_percent;
    return true;
}

uint8_t PwmHalGetDuty(gpio_t gpio){
    int slot = PwmFindSlot(gpio);
    if(slot < 0){
        return 0;
    }
    return pwm_slots[slot].duty_percent;
}

uint8_t PwmHalOff(gpio_t gpio){
    return PwmHalSetDuty(gpio, 0);
}

uint8_t PwmHalOn(gpio_t gpio){
    return PwmHalSetDuty(gpio, 100);
}

/*==================[end of file]============================================*/
