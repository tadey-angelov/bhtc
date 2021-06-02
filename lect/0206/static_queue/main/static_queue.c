#include <stdio.h>

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

void prod(void *_arg) {
	QueueHandle_t *q = (QueueHandle_t *)_arg;
	unsigned int val = 0u;
	int j;
	
	(void)_arg;
	
	while (1) {
		for (j = 0; j < 32; j++, val++)
			xQueueSend(*q, (void *)&val, portMAX_DELAY);
		vTaskDelay(1000u / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}

void cons(void *_arg) {
	QueueHandle_t *q = (QueueHandle_t *)_arg;
	unsigned int val;
	
	(void)_arg;
	
	while (1) {
		xQueueReceive(*q, &val, portMAX_DELAY);
		printf("%u\n", val);
	}
	
	vTaskDelete(NULL);
}

void app_main(void) {
	static TaskHandle_t producer_hnd, consumer_hnd;
	static QueueHandle_t queue_hnd;
	static StaticTask_t producer, consumer;
	static StaticQueue_t queue;
	static StackType_t producer_stack[4u * configMINIMAL_STACK_SIZE];
	static StackType_t consumer_stack[4u * configMINIMAL_STACK_SIZE];
	static uint8_t queue_buffer[16u * sizeof (unsigned int)];
	int i;
		
	queue_hnd = xQueueCreateStatic(16, sizeof (unsigned int), queue_buffer, &queue);
	
	producer_hnd = xTaskCreateStatic(
			prod,
			"producer_task",
			4u * configMINIMAL_STACK_SIZE,
			&queue_hnd,
			tskIDLE_PRIORITY + 1u,
			producer_stack,
			&producer
	);
	
	consumer_hnd = xTaskCreateStatic(
			cons,
			"consumer_task",
			4u * configMINIMAL_STACK_SIZE,
			&queue_hnd,
			tskIDLE_PRIORITY + 2u,
			consumer_stack,
			&consumer
	);
	
	(void)queue_hnd;
	(void)producer_hnd;
	(void)consumer_hnd;
	
	for (i = 0u; 1; i++) { /* house keeping */
		printf("house keeping...\n");
		vTaskDelay(10000u / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}

