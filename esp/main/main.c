#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "cJSON.h"

#define TXD_PIN 1
#define RXD_PIN 0
#define UART_PORT UART_NUM_1
#define BUF_SIZE 1024

void serial_json_task(void *arg) {
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

    uint8_t *rx_buf = (uint8_t *) malloc(BUF_SIZE);
    int tick = 0;

    while (1) {
        // 1. Construct and send JSON
        cJSON *tx_json = cJSON_CreateObject();
        cJSON_AddStringToObject(tx_json, "device", "esp_coprocessor");
        cJSON_AddStringToObject(tx_json, "action", "ping");
        cJSON_AddNumberToObject(tx_json, "tick", tick++);
        
        char *json_str = cJSON_PrintUnformatted(tx_json);
        uart_write_bytes(UART_PORT, json_str, strlen(json_str));
        uart_write_bytes(UART_PORT, "\n", 1); // Newline delimiter for Python readline()
        
        free(json_str);
        cJSON_Delete(tx_json);

        // 2. Listen for response from Penny
        int len = uart_read_bytes(UART_PORT, rx_buf, BUF_SIZE - 1, 1000 / portTICK_PERIOD_MS);
        if (len > 0) {
            rx_buf[len] = '\0';
            
            cJSON *rx_json = cJSON_Parse((char *)rx_buf);
            if (rx_json != NULL) {
                cJSON *status = cJSON_GetObjectItem(rx_json, "status");
                if (cJSON_IsString(status)) {
                    // Output to standard console for debugging
                    printf("Penny acknowledged: %s\n", status->valuestring); 
                }
                cJSON_Delete(rx_json);
            }
        }
        
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void app_main(void) {
    printf("2026 venykrid. ESP32 starting...\n");
    xTaskCreate(serial_json_task, "serial_json_task", 4096, NULL, 10, NULL);
}
