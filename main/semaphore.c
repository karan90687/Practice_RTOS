#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

// Pin definitions
#define BUTTON_PIN GPIO_NUM_0   // Boot button on most ESP32 boards
#define LED_PIN    GPIO_NUM_2   // Built-in LED

// Global semaphore handle
SemaphoreHandle_t button_semaphore = NULL;
// ISR function - MUST be marked with IRAM_ATTR
void IRAM_ATTR button_isr_handler(void *arg)
{
    static uint32_t isr_call_count = 0;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    isr_call_count++;  // Count how many times ISR is triggered
    
    // Give the semaphore (signal the task)
    xSemaphoreGiveFromISR(button_semaphore, &xHigherPriorityTaskWoken);
    
    // If giving semaphore woke a higher priority task, request context switch
    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

// LED task - waits for button press via semaphore
void led_task(void *pvParameters)
{
    int activation_count = 0;
    printf("LED Task started. Waiting for button press...\n");
    
    while (1) {
        // Wait for semaphore (blocks here until ISR gives it)
        if (xSemaphoreTake(button_semaphore, portMAX_DELAY) == pdTRUE) {
            activation_count++;
            
            uint32_t start_time = xTaskGetTickCount();
            printf("\n=== ACTIVATION #%d ===\n", activation_count);
            printf("Button pressed at time: %lu ms\n", start_time * portTICK_PERIOD_MS);
            printf("Turning LED ON\n");
            
            // Turn LED ON
            gpio_set_level(LED_PIN, 1);
            
            // Keep it ON for 2 seconds
            vTaskDelay(pdMS_TO_TICKS(2000));
            
            // Turn LED OFF
            gpio_set_level(LED_PIN, 0);
            
            uint32_t end_time = xTaskGetTickCount();
            printf("LED turned OFF at time: %lu ms\n", end_time * portTICK_PERIOD_MS);
            printf("Total duration: %lu ms\n", (end_time - start_time) * portTICK_PERIOD_MS);
            printf("Waiting for next press...\n\n");
        }
    }
}
void app_main(void)
{
    // Step 1: Create the binary semaphore
    button_semaphore = xSemaphoreCreateBinary();
    
    if (button_semaphore == NULL) {
        printf("Failed to create semaphore!\n");
        return;
    }
    printf("System initialized. Press BOOT button to test.\n");
    // Step 2: Configure GPIOs
    gpio_config_t io_conf = {0};
    // Configure button pin as input with pull-up and interrupt on falling edge
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << BUTTON_PIN);
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);      
    // Configure LED pin as output
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << LED_PIN);
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);  
    // Step 3: Install ISR service and add ISR handler for button pin
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);                                     
    // Step 4: Create the LED task
    xTaskCreate(led_task, "led_task", 2048, NULL, 10, NULL);    
    // Main task does nothing, everything is handled in led_task and ISR
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));

    }
}



// When you press the button multiple times quickly:

// First press → LED ON for 2s
// Second press (during those 2s) → Queued
// LED OFF → Immediately ON again (looks like it stayed on for 4s!)

// From your perspective: "LED stayed on for 4 seconds"
// From system's perspective: "LED was ON for 2s, OFF for 1ms, ON for 2s" (two separate activations)