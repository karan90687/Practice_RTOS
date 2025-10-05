#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"

#define LED_PIN   GPIO_NUM_2
#define FSR_PIN GPIO_NUM_25

TaskHandle_t ledtask1_handle = NULL;
TaskHandle_t ledtask2_handle = NULL;
TaskHandle_t ledtask3_handle = NULL;

void ledtask1(void *pvParameters)
{
    while (1) {
        printf("ledtask1 is running\r\n");
        gpio_set_level(LED_PIN, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(LED_PIN, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void ledtask2(void *pvParameters)
{
    while (1) {
        printf("ledtask2 is running\r\n");
        gpio_set_level(LED_PIN, 1);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        gpio_set_level(LED_PIN, 0);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void ledtask3(void *pvParameters)
{
    while (1) {
        printf("ledtask3 is running\r\n");
        gpio_set_level(LED_PIN, 1);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        gpio_set_level(LED_PIN, 0);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void supervisor_task(void *pvParameters)
{
    gpio_set_direction(FSR_PIN, GPIO_MODE_INPUT);
    adc1_config_width(ADC_WIDTH_BIT_12); 
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11); 

    while (1) {
        int raw = adc1_get_raw(ADC1_CHANNEL_6);

        printf("FSR raw value: %d\n", raw);

        if (raw > 100) {
            // run only ledtask3
            if (ledtask1_handle) { vTaskDelete(ledtask1_handle); ledtask1_handle = NULL; }
            if (ledtask2_handle) { vTaskDelete(ledtask2_handle); ledtask2_handle = NULL; }
            if (ledtask3_handle == NULL) {
                xTaskCreate(ledtask3, "ledtask3", 2048, NULL, 3, &ledtask3_handle);
            }
        } else {
            // run ledtask1 and ledtask2
            if (ledtask3_handle) { vTaskDelete(ledtask3_handle); ledtask3_handle = NULL; }
            if (ledtask1_handle == NULL) {
                xTaskCreate(ledtask1, "ledtask1", 2048, NULL, 8, &ledtask1_handle);
            }
            if (ledtask2_handle == NULL) {
                xTaskCreate(ledtask2, "ledtask2", 2048, NULL, 9, &ledtask2_handle);
            }
        }

        vTaskDelay(200 / portTICK_PERIOD_MS); // check condition every 200ms
    }
}

void app_main(void)
{
        gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    xTaskCreate(supervisor_task, "supervisor", 2048, NULL, 10, NULL);
}
