#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "cJSON.h"
#include "driver/ledc.h"

#define HOST "polly"

#define TXD_PIN 16
#define RXD_PIN 17
#define UART_PORT UART_NUM_1
#define BUF_SIZE 1024

#define LED_R_PIN 0
#define LED_G_PIN 1
#define LED_B_PIN 2

#define BOOTANI_SPEED 500


void init_uart() {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    
    uart_param_config(UART_PORT, &uart_config);
    uart_set_pin(UART_PORT, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0);
}

void init_rgb() {
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_conf);

    int pins[] = {LED_R_PIN, LED_G_PIN, LED_B_PIN};
    for (int i = 0; i < 3; i++) {
        ledc_channel_config_t ch_conf = {
            .gpio_num = pins[i],
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = i,
            .timer_sel = LEDC_TIMER_0,
            .duty = 0,
            .hpoint = 0
        };
        ledc_channel_config(&ch_conf);
    }
    ledc_fade_func_install(0);
}

void status_led_task(void *arg) {
    int status_code = (int)arg;
    
    int r_val = 0, g_val = 0, b_val = 0;

    switch (status_code) {
        case 0: g_val = 255; break;                             // Green (Idle)
        case 1: r_val = 255; break;                             // Red (Generic Error)
        case 2: b_val = 255; break;                             // Blue (Wifi)
        case 3: r_val = 255; g_val = 255; break;                // Yellow (Warning)
        case 4: r_val = 255; b_val = 255; break;                // Magenta
        case 5: g_val = 255; b_val = 255; break;                // Cyan
        case 6: break;                                          // Sleep (off)
        default: r_val = 255; g_val = 255; b_val = 255; break;  // White (Unknown code)
    }

    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, 0, r_val, 500); 
    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, 1, g_val, 500); 
    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, 2, b_val, 500); 
    
    ledc_fade_start(LEDC_LOW_SPEED_MODE, 0, LEDC_FADE_NO_WAIT);
    ledc_fade_start(LEDC_LOW_SPEED_MODE, 1, LEDC_FADE_NO_WAIT);
    ledc_fade_start(LEDC_LOW_SPEED_MODE, 2, LEDC_FADE_NO_WAIT);
    
    vTaskDelay(700 / portTICK_PERIOD_MS);
    
    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, 0, 0, 500);
    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, 1, 0, 500);
    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, 2, 0, 500);
    
    ledc_fade_start(LEDC_LOW_SPEED_MODE, 0, LEDC_FADE_NO_WAIT);
    ledc_fade_start(LEDC_LOW_SPEED_MODE, 1, LEDC_FADE_NO_WAIT);
    ledc_fade_start(LEDC_LOW_SPEED_MODE, 2, LEDC_FADE_NO_WAIT);
    
    vTaskDelay(500 / portTICK_PERIOD_MS);

    vTaskDelete(NULL);
}

void startup_anim_task(void *arg) {
    // Tune these bases to get your perfect "pastel white" resting color
    int r_base = 220; // Red needs high power to compete
    int g_base = 100; // Green is overpowering, throttle it down heavily
    int b_base = 150; // Blue sits somewhere in the middle

    // Tune these peaks for the active color sweep
    int r_peak = 255; // Keep red at absolute max
    int g_peak = 150; // Keep green's peak relatively low
    int b_peak = 210; 
    
    // 0. Initialize by quickly fading from OFF to Pastel Red (500ms)
    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, 0, r_peak, 500); 
    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, 1, g_base, 500); 
    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, 2, b_base, 500); 
    ledc_fade_start(LEDC_LOW_SPEED_MODE, 0, LEDC_FADE_NO_WAIT);
    ledc_fade_start(LEDC_LOW_SPEED_MODE, 1, LEDC_FADE_NO_WAIT);
    ledc_fade_start(LEDC_LOW_SPEED_MODE, 2, LEDC_FADE_NO_WAIT);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    // Loop 3 times (3 seconds per cycle)
    for (int i = 0; i < 3; i++) {
        
        // 1. Fade to Pastel Green
        ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, 0, r_base, BOOTANI_SPEED); 
        ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, 1, g_peak, BOOTANI_SPEED); 
        ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, 2, b_base, BOOTANI_SPEED); 
        ledc_fade_start(LEDC_LOW_SPEED_MODE, 0, LEDC_FADE_NO_WAIT);
        ledc_fade_start(LEDC_LOW_SPEED_MODE, 1, LEDC_FADE_NO_WAIT);
        ledc_fade_start(LEDC_LOW_SPEED_MODE, 2, LEDC_FADE_NO_WAIT);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        // 2. Fade to Pastel Blue
        ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, 0, r_base, BOOTANI_SPEED); 
        ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, 1, g_base, BOOTANI_SPEED); 
        ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, 2, b_peak, BOOTANI_SPEED); 
        ledc_fade_start(LEDC_LOW_SPEED_MODE, 0, LEDC_FADE_NO_WAIT);
        ledc_fade_start(LEDC_LOW_SPEED_MODE, 1, LEDC_FADE_NO_WAIT);
        ledc_fade_start(LEDC_LOW_SPEED_MODE, 2, LEDC_FADE_NO_WAIT);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        // 3. Fade back to Pastel Red OR Fade to OFF
        if (i < 2) {
            ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, 0, r_peak, BOOTANI_SPEED); 
            ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, 1, g_base, BOOTANI_SPEED); 
            ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, 2, b_base, BOOTANI_SPEED); 
        } else {
            ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, 0, 0, BOOTANI_SPEED);
            ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, 1, 0, BOOTANI_SPEED);
            ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, 2, 0, BOOTANI_SPEED);
        }
        ledc_fade_start(LEDC_LOW_SPEED_MODE, 0, LEDC_FADE_NO_WAIT);
        ledc_fade_start(LEDC_LOW_SPEED_MODE, 1, LEDC_FADE_NO_WAIT);
        ledc_fade_start(LEDC_LOW_SPEED_MODE, 2, LEDC_FADE_NO_WAIT);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
    vTaskDelete(NULL); 
}

void ping_task(void *arg) {
    int tick = 0;

    while (1) {
        cJSON *tx_json = cJSON_CreateObject();
        cJSON_AddStringToObject(tx_json, "device", HOST);
        cJSON_AddStringToObject(tx_json, "action", "ping");
        cJSON_AddNumberToObject(tx_json, "tick", tick++);
        
        char *json_str = cJSON_PrintUnformatted(tx_json);
        uart_write_bytes(UART_PORT, json_str, strlen(json_str));
        uart_write_bytes(UART_PORT, "\n", 1); 
        
        free(json_str);
        cJSON_Delete(tx_json);

        // Put this specific task to sleep for 30 seconds
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

void uart_rx_task(void *arg) {
    uint8_t *rx_buf = (uint8_t *) malloc(BUF_SIZE);
    static char line_buf[BUF_SIZE];
    int line_pos = 0;

    while (1) {
        // Blocks for up to 100ms waiting for data, yields CPU to other tasks while waiting
        int len = uart_read_bytes(UART_PORT, rx_buf, BUF_SIZE - 1, 100 / portTICK_PERIOD_MS);
        
        if (len > 0) {
            for (int i = 0; i < len; i++) {
                if (rx_buf[i] == '\n') {
                    line_buf[line_pos] = '\0'; 
                    
                    cJSON *rx_json = cJSON_Parse(line_buf);
                    if (rx_json != NULL) {
                        cJSON *action = cJSON_GetObjectItem(rx_json, "action");
                        if (cJSON_IsString(action) && (action->valuestring != NULL)) {
                            // Status Report
                            if (strcmp(action->valuestring, "status") == 0) {
                                cJSON *status = cJSON_GetObjectItem(rx_json, "status");
                                if (cJSON_IsNumber(status)) {
                                    xTaskCreate(status_led_task, "ping_anim", 2048, (void *)status->valueint, 5, NULL); 
                                }
                            }

                            // Startup Animation
                            if (strcmp(action->valuestring, "play-bootanimation") == 0) {
                                xTaskCreate(startup_anim_task, "startup_anim", 2048, NULL, 5, NULL); 
                            }
                        }
                        cJSON_Delete(rx_json);
                    }
                    line_pos = 0; 
                } else if (line_pos < BUF_SIZE - 1) {
                    line_buf[line_pos++] = rx_buf[i]; 
                }
            }
        }
    }
}

void app_main(void) {
    printf("2026 venykrid. ESP32 starting...\n");
    init_rgb();
    init_uart();

    xTaskCreate(uart_rx_task, "uart_rx_task", 4096, NULL, 10, NULL);
    xTaskCreate(ping_task, "ping_task", 4096, NULL, 9, NULL);
}
