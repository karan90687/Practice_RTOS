#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"

#define LED_PIN   GPIO_NUM_2
#define FSR_PIN  GPIO_NUM_25

TaskHandle_t ledtask1_handle = NULL;
TaskHandle_t ledtask2_handle = NULL;
TaskHandle_t ledtask3_handle = NULL;

// LED toggle helper
void toggle_led(uint32_t delay_ms) {
    gpio_set_level(LED_PIN, 1);
    vTaskDelay(delay_ms / portTICK_PERIOD_MS);
    gpio_set_level(LED_PIN, 0);
    vTaskDelay(delay_ms / portTICK_PERIOD_MS);
}

void ledtask1(void *pvParameters) {
    while (1) {
        printf("ledtask1 running\n");
        toggle_led(1000);
    }
}

void ledtask2(void *pvParameters) {
    while (1) {
        printf("ledtask2 running\n");
        toggle_led(5000);
    }
}

void ledtask3(void *pvParameters) {
    while (1) {
        printf("ledtask3 running\n");
        toggle_led(2000);
    }
}

void supervisor_task(void *pvParameters) {
    // Configure FSR once
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);

    while (1) {
        int raw = adc1_get_raw(ADC1_CHANNEL_6);
        printf("FSR raw value: %d\n", raw);

        if (raw > 100) {
            // Only ledtask3 active
            if (ledtask1_handle) vTaskSuspend(ledtask1_handle);
            if (ledtask2_handle) vTaskSuspend(ledtask2_handle);
            if (ledtask3_handle) vTaskResume(ledtask3_handle);
        } else {
            // ledtask1 and ledtask2 active
            if (ledtask3_handle) vTaskSuspend(ledtask3_handle);
            if (ledtask1_handle) vTaskResume(ledtask1_handle);
            if (ledtask2_handle) vTaskResume(ledtask2_handle);
        }

        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

void app_main(void) {
    // Configure GPIO once
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    // Create all tasks once
    xTaskCreate(ledtask1, "ledtask1", 2048, NULL, 8, &ledtask1_handle);
    xTaskCreate(ledtask2, "ledtask2", 2048, NULL, 9, &ledtask2_handle);
    xTaskCreate(ledtask3, "ledtask3", 2048, NULL, 3, &ledtask3_handle);

    // Initially suspend ledtask3
    if (ledtask3_handle) vTaskSuspend(ledtask3_handle);

    xTaskCreate(supervisor_task, "supervisor", 2048, NULL, 10, NULL);
}
