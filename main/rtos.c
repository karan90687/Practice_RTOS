#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "freertos/queue.h"
#include "driver/adc.h"
#include "esp_random.h"

#define uxQueueLength (1) // Number of items the queue can hold at a time 
#define uxItemSize sizeof(int)
#define FSR_PIN  GPIO_NUM_32
#define LED_PIN  GPIO_NUM_2

QueueHandle_t myQueue;
TaskHandle_t producer_handle = NULL;
TaskHandle_t consumer_handle = NULL; 
TaskHandle_t producer_handle2 = NULL;

void producer_task(void *params) {
    while (1) {
        int raw = adc1_get_raw(ADC1_CHANNEL_4);
        printf("Produced: %d\n", raw);

        vTaskDelay(pdMS_TO_TICKS(10));
        xQueueSend(myQueue, &raw, portMAX_DELAY);
    }
}

 void producer_task2(void *params) {
    int lost_count = 0;
    while (1) {
      int raw = adc1_get_raw(ADC1_CHANNEL_4);
        
        // Non-blocking send with 0 timeout
        if (xQueueSend(myQueue, &raw, 0) == pdPASS) {
            printf("✓ Producer1 sent: %d\n", raw);
        } else {
            lost_count++;
            printf("❌ Producer1 LOST data! (Total lost: %d)\n", lost_count);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void consumer_task(void *params) {
    while (1) {
        int received_value;
          // Check queue status
        UBaseType_t items_waiting = uxQueueMessagesWaiting(myQueue);
        UBaseType_t spaces_available = uxQueueSpacesAvailable(myQueue);
        
        printf("Queue: %d items waiting, %d spaces free\n", items_waiting, spaces_available);

        if (xQueueReceive(myQueue, &received_value, 0) == pdPASS) {
            printf("Consumed: %d\n", received_value);
            if (received_value > 2000) {
                gpio_set_level(LED_PIN, 1);
            } else {
                gpio_set_level(LED_PIN, 0);
            }
        }
        // Delay happens EVERY loop iteration
        vTaskDelay(pdMS_TO_TICKS(1000));
        xQueuePeek(myQueue, &received_value, 0);
    }
}

void app_main(void) {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    
    myQueue = xQueueCreate(uxQueueLength, uxItemSize);
    xTaskCreatePinnedToCore(producer_task, "Producer", 2048, NULL, 5, &producer_handle,0);
    xTaskCreate(consumer_task, "Consumer", 2048, NULL, 5, &consumer_handle);
    xTaskCreatePinnedToCore(producer_task2, "Producer2", 2048, NULL, 5, &producer_handle2,1);
   
}