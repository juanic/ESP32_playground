#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "touch_hal.h"

#define TEST_PAD            TOUCH_PAD_8
#define ARBITRARY_THRESHOLD 1000
#define SWEEP_DELAY_MS      500
#define PLOT_HEIGHT         18
#define STEP_COUNT          8
#define CALIB_SWEEPS        3
#define CMD_BUF_SIZE        32

static const char *TAG = "2_touch_sweep";

typedef enum {
    MODE_PLOT,
    MODE_STAT
} run_mode_t;

typedef struct {
    touch_charge_speed_t charge_speed;
    touch_init_charge_volt_t init_charge_volt;
    touch_volt_lim_l_t volt_lim_l;
    touch_volt_lim_h_t volt_lim_h;
} sweep_step_t;

static sweep_step_t sweep_steps[STEP_COUNT];
static uint32_t fixed_max_val = 0;
static uint32_t fixed_min_val = 0;
static uint32_t graph_max = 0;
static uint32_t graph_min = 0;
static run_mode_t current_mode = MODE_STAT;

static void init_sweep_table(void) {
    /*
     * 8 configuraciones seleccionadas para diversidad de sensibilidad.
     * volt_lim: fijo 0.5V–2.5V (swing máximo, mayor S/N).
     * charge_duration_ms: fijo 5ms (configurado en TouchHalSetSampleConfig).
     *
     * Pasos 0–5: extremos de charge_speed (1 y 7) × las 3 init_charge_volt.
     * Paso 6: punto medio (speed=4, FLOAT) como referencia intermedia.
     * Paso 7: replicación de la config default del HAL para comparar contra baseline.
     */
    static const sweep_step_t table[STEP_COUNT] = {
        /* speed  init_volt             lim_l           lim_h */
        {  1,     TOUCH_INIT_CHARGE_VOLT_LOW,   TOUCH_VOLT_LIM_L_0V5, TOUCH_VOLT_LIM_H_2V5 },  /* 0: mín sensibilidad, voltaje inicial LOW  */
        {  1,     TOUCH_INIT_CHARGE_VOLT_FLOAT, TOUCH_VOLT_LIM_L_0V5, TOUCH_VOLT_LIM_H_2V5 },  /* 1: mín sensibilidad, FLOAT (pin en hi-Z)  */
        {  1,     TOUCH_INIT_CHARGE_VOLT_HIGH,  TOUCH_VOLT_LIM_L_0V5, TOUCH_VOLT_LIM_H_2V5 },  /* 2: mín sensibilidad, HIGH                 */
        {  7,     TOUCH_INIT_CHARGE_VOLT_LOW,   TOUCH_VOLT_LIM_L_0V5, TOUCH_VOLT_LIM_H_2V5 },  /* 3: máx sensibilidad, LOW                  */
        {  7,     TOUCH_INIT_CHARGE_VOLT_FLOAT, TOUCH_VOLT_LIM_L_0V5, TOUCH_VOLT_LIM_H_2V5 },  /* 4: máx sensibilidad, FLOAT                */
        {  7,     TOUCH_INIT_CHARGE_VOLT_HIGH,  TOUCH_VOLT_LIM_L_0V5, TOUCH_VOLT_LIM_H_2V5 },  /* 5: máx sensibilidad, HIGH                 */
        {  4,     TOUCH_INIT_CHARGE_VOLT_FLOAT, TOUCH_VOLT_LIM_L_0V5, TOUCH_VOLT_LIM_H_2V5 },  /* 6: punto medio, referencia intermedia     */
        {  7,     TOUCH_INIT_CHARGE_VOLT_DEFAULT,TOUCH_VOLT_LIM_L_0V5,TOUCH_VOLT_LIM_H_2V5 },  /* 7: default HAL (speed=7, FLOAT), baseline */
    };
    memcpy(sweep_steps, table, sizeof(table));
}

static void print_ascii_graph(const uint32_t *raw_values) {
    printf("\n--- ASCII SWEEP PLOT (%d steps, Fixed Scale) ---\n", STEP_COUNT);
    uint32_t range = graph_max - graph_min;
    if (range == 0) range = 1;

    for (int r = PLOT_HEIGHT - 1; r >= 0; r--) {
        printf("|");
        for (int c = 0; c < STEP_COUNT; c++) {
            uint32_t val = raw_values[c];
            if (val > graph_max) val = graph_max;
            if (val < graph_min) val = graph_min;
            int row_idx = (int)(((float)(val - graph_min) / range) * (PLOT_HEIGHT - 1) + 0.5f);
            printf("%c", (row_idx == r) ? '_' : ' ');
        }
        printf("|\n");
    }
    printf("+");
    for (int c = 0; c < STEP_COUNT; c++) printf("-");
    printf("+\n");
}

static void compute_and_print_stats(const uint32_t *raw_values) {
    double sum = 0.0;
    double sum_sq = 0.0;
    uint32_t min_val = 0xFFFFFFFF;
    uint32_t max_val = 0;

    for (int i = 0; i < STEP_COUNT; i++) {
        double v = (double)raw_values[i];
        sum += v;
        sum_sq += v * v;
        if (raw_values[i] < min_val) min_val = raw_values[i];
        if (raw_values[i] > max_val) max_val = raw_values[i];
    }

    double mean = sum / STEP_COUNT;
    double variance = (sum_sq / STEP_COUNT) - (mean * mean);
    if (variance < 0.0) variance = 0.0;
    double stddev = sqrt(variance);
    double cv = (mean != 0.0) ? (stddev / mean * 100.0) : 0.0;

    printf("\n--- SWEEP STATS (%d steps) ---\n", STEP_COUNT);
    printf("Mean: %.1f | StdDev: %.1f | Min: %lu | Max: %lu | CV: %.2f%%\n",
           mean, stddev, (unsigned long)min_val, (unsigned long)max_val, cv);
}

static void command_reader_task(void *arg) {
    char buf[CMD_BUF_SIZE];
    while (true) {
        if (fgets(buf, sizeof(buf), stdin) != NULL) {
            buf[strcspn(buf, "\r\n")] = '\0';
            if (strcmp(buf, "#PLOT") == 0) {
                current_mode = MODE_PLOT;
                ESP_LOGI(TAG, "Mode switched to PLOT");
            } else if (strcmp(buf, "#STAT") == 0) {
                current_mode = MODE_STAT;
                ESP_LOGI(TAG, "Mode switched to STAT");
            } else if (strlen(buf) > 0) {
                ESP_LOGW(TAG, "Unknown command: '%s' (use #PLOT or #STAT)", buf);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Initializing Touch HAL for Sweep Test...");

    if (!TouchHalInit()) {
        ESP_LOGE(TAG, "TouchHalInit failed!");
        return;
    }

    /* Configurar controller UNA sola vez: ventana fija 5ms, swing máximo 0.5V–2.5V */
    if (!TouchHalSetSampleConfig(5.0f, TOUCH_VOLT_LIM_L_0V5, TOUCH_VOLT_LIM_H_2V5)) {
        ESP_LOGE(TAG, "TouchHalSetSampleConfig failed!");
        return;
    }

    if (!TouchHalChannelConfig(TEST_PAD, ARBITRARY_THRESHOLD)) {
        ESP_LOGE(TAG, "TouchHalChannelConfig failed for pad %d!", TEST_PAD);
        return;
    }

    init_sweep_table();
    ESP_LOGI(TAG, "Sweep: 8 configs (speed 1/4/7 × init_volt LOW/FLOAT/HIGH + default), fixed 5ms, 0.5V–2.5V.");

    xTaskCreate(command_reader_task, "cmd_reader", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "Commands: #PLOT (graph) | #STAT (statistics)");

    uint32_t raw_values[STEP_COUNT];

    ESP_LOGI(TAG, "Starting calibration (%d sweeps)...", CALIB_SWEEPS);
    uint32_t calib_max = 0;
    uint32_t calib_min = 0xFFFFFFFF;

    for (int s = 0; s < CALIB_SWEEPS; s++) {
        vTaskDelay(pdMS_TO_TICKS(100));
        for (int i = 0; i < STEP_COUNT; i++) {
            uint32_t val = 0;
            bool ok = TouchHalSweepExcite(
                TEST_PAD,
                sweep_steps[i].charge_speed,
                sweep_steps[i].init_charge_volt,
                sweep_steps[i].volt_lim_l,
                sweep_steps[i].volt_lim_h,
                1000,
                &val
            );
            if (ok) {
                if (val > calib_max) calib_max = val;
                if (val < calib_min) calib_min = val;
            }
        }
    }

    fixed_max_val = calib_max;
    fixed_min_val = calib_min;
    graph_max = fixed_max_val;
    graph_min = (uint32_t)(fixed_min_val * 0.5f);

    ESP_LOGI(TAG, "Calibration done! Baseline Min: %lu, Max: %lu",
             (unsigned long)fixed_min_val, (unsigned long)fixed_max_val);
    ESP_LOGI(TAG, "Graph scale: Min: %lu, Max: %lu",
             (unsigned long)graph_min, (unsigned long)graph_max);

    while (true) {
        for (int i = 0; i < STEP_COUNT; i++) {
            bool ok = TouchHalSweepExcite(
                TEST_PAD,
                sweep_steps[i].charge_speed,
                sweep_steps[i].init_charge_volt,
                sweep_steps[i].volt_lim_l,
                sweep_steps[i].volt_lim_h,
                1000,
                &raw_values[i]
            );
            if (!ok) {
                ESP_LOGE(TAG, "TouchHalSweepExcite failed at step %d!", i);
                raw_values[i] = 0;
            }
        }

        if (current_mode == MODE_PLOT) {
            print_ascii_graph(raw_values);

            uint32_t cur_min = 0xFFFFFFFF;
            uint32_t cur_max = 0;
            for (int i = 0; i < STEP_COUNT; i++) {
                if (raw_values[i] < cur_min) cur_min = raw_values[i];
                if (raw_values[i] > cur_max) cur_max = raw_values[i];
            }
            printf("Current Min: %lu | Max: %lu (Baseline Min: %lu, Max: %lu)\n",
                   (unsigned long)cur_min, (unsigned long)cur_max,
                   (unsigned long)fixed_min_val, (unsigned long)fixed_max_val);

            printf("raw:");
            for (int i = 0; i < STEP_COUNT; i++) {
                printf("%lu%s", (unsigned long)raw_values[i], (i == STEP_COUNT - 1) ? "" : ",");
            }
            printf("\n");

        } else if (current_mode == MODE_STAT) {
            compute_and_print_stats(raw_values);

            printf("raw:");
            for (int i = 0; i < STEP_COUNT; i++) {
                printf("%lu%s", (unsigned long)raw_values[i], (i == STEP_COUNT - 1) ? "" : ",");
            }
            printf("\n");
        }

        vTaskDelay(pdMS_TO_TICKS(SWEEP_DELAY_MS));
    }
}
