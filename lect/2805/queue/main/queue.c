#include <stdio.h>

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

QueueHandle_t q;

void prod(void *_arg) {
	unsigned int val = 0u;
	int j;
	
	(void)_arg;
	
	while (1) {
		xQueueSend(q, (void *)&val, portMAX_DELAY);
		for (j = 0; j < 32; j++, val++)
			xQueueSend(q, (void *)&val, portMAX_DELAY);
		vTaskDelay(1000u / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}

void cons(void *_arg) {
	unsigned long val;
	
	(void)_arg;
	
	while (1) {
		xQueueReceive(q, &val, portMAX_DELAY);
		printf("%lu\n", val);
	}
	
	vTaskDelete(NULL);
}

void app_main(void) {
	BaseType_t ret;
	TaskHandle_t p;
	TaskHandle_t c;	
	unsigned int i;
	
	q = xQueueCreate(16, sizeof (unsigned int));
	if (!q) {
		printf("cannot create queue\n");
		goto ERR;
	}
	
	ret = xTaskCreate(prod, "producer", 2u * configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &p);
	if (ret != pdPASS) {
		printf("error creating producer\n");
		goto PROD_ERR;
	}
	
	ret = xTaskCreate(cons, "consumer", 2u * configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &c);
	if (ret != pdPASS) {
		printf("error creating consumer\n");
		goto CONS_ERR;
	}
	
	for (i = 0u; 1; i++) {
		vTaskDelay(10000u / portTICK_PERIOD_MS);
	}
	
CONS_ERR:
	vTaskDelete(&p);
PROD_ERR:
	vQueueDelete(q);
ERR:
	vTaskDelete(NULL);
}

