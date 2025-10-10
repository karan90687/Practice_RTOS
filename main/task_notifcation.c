#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// Pin definitions
#define BUTTON_PIN GPIO_NUM_0   // Boot button
#define LED_PIN    GPIO_NUM_2   // Built-in LED

// Task handle - MUST be global so ISR can access it
TaskHandle_t led_task_handle = NULL;

// ISR function - sends notification instead of semaphore
void IRAM_ATTR button_isr_handler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    // Notify the LED task (increment notification value by 1)
    vTaskNotifyGiveFromISR(led_task_handle, &xHigherPriorityTaskWoken);
    
    // Request context switch if needed
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// LED task - waits for notification
void led_task(void *pvParameters)
{
    uint32_t notification_count = 0;
    
    printf("LED Task started. Waiting for button press...\n");
    
    while (1) {
        // Wait for notification (blocks here until ISR notifies)
        // This clears the notification count and returns the value
        notification_count = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        if (notification_count > 0) {
            printf("\n=== Button Pressed! (Total count: %lu) ===\n", notification_count);
            printf("Blinking LED 3 times...\n");
            
            // Blink 3 times
            for (int i = 1; i <= 3; i++) {
                printf("  Blink %d/3\n", i);
                gpio_set_level(LED_PIN, 1);
                vTaskDelay(pdMS_TO_TICKS(200));
                gpio_set_level(LED_PIN, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            
            printf("Done! Waiting for next press...\n\n");
        }
    }
}

void app_main(void)
{
    printf("System initializing...\n");
    
    // Step 1: Configure button GPIO with interrupt
    gpio_config_t io_conf = {0};
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << BUTTON_PIN);
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);
    
    // Step 2: Configure LED GPIO
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << LED_PIN);
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);
    gpio_set_level(LED_PIN, 0);
    
    // Step 3: Create LED task and SAVE its handle
    xTaskCreate(led_task, "LED_Task", 2048, NULL, 5, &led_task_handle);
    
    // Step 4: Install ISR service and attach handler
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);
    
    printf("System ready! Press BOOT button to test.\n");
    
    // Main task idle loop
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}