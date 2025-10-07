 #include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "freertos/queue.h"
#include "driver/adc.h"
#include "esp_random.h"

TaskHandle_t task1_handle = NULL;
TaskHandle_t task2_handle = NULL;
TaskHandle_t task3_handle = NULL;
SemaphoreHandle_t xMutex;
void task1(void *pvParameters)
{
    while(1)
    {
        xSemaphoreTake(xMutex, portMAX_DELAY);
        printf("Task 1 is running, hey i am karan here i am practicing rtos concepts in order to sharpen my skills in embedded system and IoT. The goal is not to devlop a code for any random thing the goal is to maek a product which solves any problem of the society so that i will be proud to be an engineer.\n");
        xSemaphoreGive(xMutex);
        vTaskDelay(pdMS_TO_TICKS(10)); // Delay for 1000 ms
    }
}
void task2(void *pvParameters)
{
    while(1)
    {
        xSemaphoreTake(xMutex, portMAX_DELAY);
        printf("Task 2 is running, hey i am karan here i am practicing rtos concepts in order to sharpen my skills in embedded system and IoT. The goal is not to devlop a code for any random thing the goal is to maek a product which solves any problem of the society so that i will be proud to be an engineer.\n");
        xSemaphoreGive(xMutex);
        vTaskDelay(pdMS_TO_TICKS(10)); // Delay for 1000 ms
        
    }
}
void task3(void *pvParameters)
{
    while(1)
    {
        xSemaphoreTake(xMutex, portMAX_DELAY);
        printf("Task 3 is running, hey i am karan here i am practicing rtos concepts in order to sharpen   my skills in embedded system and IoT. The goal is not to devlop a code for any random thing the goal is to maek a product which solves any problem of the society so that i will be proud to be an engineer.\n");
        xSemaphoreGive(xMutex);

        vTaskDelay(pdMS_TO_TICKS(10)); // Delay for 1000 ms
       
    }

}
void app_main(void)
{
    xMutex = xSemaphoreCreateMutex();

    xTaskCreate(task1, "Task 1", 2048, NULL, 5, &task1_handle);
    xTaskCreate(task2, "Task 2", 2048, NULL, 5, &task2_handle);
    xTaskCreate(task3, "Task 3", 2048, NULL, 5, &task3_handle);

}
