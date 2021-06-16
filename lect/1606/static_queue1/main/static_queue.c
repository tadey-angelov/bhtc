#include <stdio.h>

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#define SIZE (16u)

static TaskHandle_t producer_hnd, consumer0_hnd, consumer1_hnd;
static StaticTask_t producer, consumer0, consumer1;
static StackType_t producer_stack[4u * configMINIMAL_STACK_SIZE];
static StackType_t consumer0_stack[4u * configMINIMAL_STACK_SIZE];
static StackType_t consumer1_stack[4u * configMINIMAL_STACK_SIZE];
static StaticQueue_t q;
static QueueHandle_t q_hnd;
static uint8_t q_buf[SIZE * sizeof (int)];

void prod(void *_arg) {
	int val = 0;
	int j;
	
	(void)_arg;
	
	while (1) {
		for (j = 0; j < 16; j++, val++) {
			printf("enqueue %d\n", val);
			xQueueSend(q_hnd, &val, portMAX_DELAY);
		}
		vTaskDelay(1000u / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}

void cons(void *_arg) {
	int val;
	unsigned int arg = (unsigned int)_arg;
	
	while (1) {
		xQueueReceive(q_hnd, &val, portMAX_DELAY);
		printf("consumption %d of %d\n", arg, val);
		vTaskDelay(arg / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}

void app_main(void) {
	unsigned int i;
	
	q_hnd = xQueueCreateStatic(SIZE, sizeof (int), q_buf, &q);
	(void)q_hnd;
	
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

