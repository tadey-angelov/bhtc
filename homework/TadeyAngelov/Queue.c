#include <stdio.h>

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#define LENGTH (16u)
#define SIZE (LENGTH* sizeof (int))

static TaskHandle_t producer_hnd, consumer0_hnd, consumer1_hnd;
static StaticTask_t producer, consumer0, consumer1;
static StackType_t producer_stack[4u * configMINIMAL_STACK_SIZE];
static StackType_t consumer0_stack[4u * configMINIMAL_STACK_SIZE];
static StackType_t consumer1_stack[4u * configMINIMAL_STACK_SIZE];
static int queue_buffer[SIZE];
static size_t in = 0u, out = 0u;
static SemaphoreHandle_t countsem_hnd, spacesem_hnd, lock_hnd;
static StaticSemaphore_t countsem, spacesem, lock;

static void enqueue(int *buffer) {
	xSemaphoreTake(spacesem_hnd, portMAX_DELAY);
	
	xSemaphoreTake(lock_hnd, portMAX_DELAY);
	memccpy(buffer, &queue_buffer[in], ((queue_buffer[(in++) & (SIZE - 1u)] )*(sizeof(queue_buffer)/SIZE)));
	xSemaphoreGive(lock_hnd);
	
	xSemaphoreGive(countsem_hnd);
	
	return;
}

static int* dequeue(void) {
	int* result;
	
	xSemaphoreTake(countsem_hnd, portMAX_DELAY);
	
	xSemaphoreTake(lock_hnd, portMAX_DELAY);
	memccpy(result, &queue_buffer[out], ((queue_buffer[(out++) & (SIZE - 1u)] )*(sizeof(queue_buffer)/SIZE)));
	xSemaphoreGive(lock_hnd);
	
	xSemaphoreGive(spacesem_hnd);
	
	return result;
}

void prod(void *_arg) {
	int val = 0;
	int j;
	
	(void)_arg;
	
	while (1) {
		for (j = 0; j < 16; j++, val++) {
			printf("enqueue %d\n", val);
			enqueue(val);
		}
		vTaskDelay(1000u / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}

void cons(void *_arg) {
	int val;
	unsigned int arg = (unsigned int)_arg;
	
	while (1) {
		val = dequeue();
		printf("consumption %d of %d\n", arg, val);
		vTaskDelay(arg / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}

void app_main(void) {
	unsigned int i;
	
	lock_hnd = xSemaphoreCreateBinaryStatic(&lock);
	xSemaphoreGive(lock_hnd);
	
	countsem_hnd = xSemaphoreCreateCountingStatic(LENGTH, 0u, &countsem);
	(void)countsem_hnd;
	
	spacesem_hnd = xSemaphoreCreateCountingStatic(LENGTH, LENGTH, &spacesem);
	(void)spacesem_hnd;
        
	producer_hnd = xTaskCreateStatic(
			prod,
			"producer_task",
			4u * configMINIMAL_STACK_SIZE,
			NULL,
			tskIDLE_PRIORITY + 8u,
			producer_stack,
			&producer
	);
	
	consumer0_hnd = xTaskCreateStatic(
			cons,
			"consumer0_task",
			4u * configMINIMAL_STACK_SIZE,
			(void *)100u,
			tskIDLE_PRIORITY,
			consumer0_stack,
			&consumer0
	);
	
	consumer1_hnd = xTaskCreateStatic(
			cons,
			"consumer1_task",
			4u * configMINIMAL_STACK_SIZE,
			(void *)10u,
			tskIDLE_PRIORITY,
			consumer1_stack,
			&consumer1
	);
	
	(void)producer_hnd;
	(void)consumer0_hnd;
	(void)consumer1_hnd;
	
	for (i = 0u; 1; i++) { /* house keeping */
		printf("housekeeping %u...\n", i);
		vTaskDelay(200u / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}