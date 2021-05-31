#include <stdio.h>

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

QueueHandle_t q;

void prod0(void *_arg) {
	unsigned int val = 0u;
	int j;
	
	(void)_arg;
	
	while (1) {
		for (j = 0; j < 32; j++, val++) {
			xQueueSend(q, (void *)&val, portMAX_DELAY);
			printf("0\t\tenqueue %u\n", val);
		}
		vTaskDelay(1000u / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}

void prod1(void *_arg) {
	unsigned int val = 1u;
	int j;
	
	(void)_arg;
	
	while (1) {
		for (j = 0; j < 8; j++, val <<= 1) {
			xQueueSend(q, (void *)&val, portMAX_DELAY);
			printf("1\t\t\tenqueue %u\n", val);
		}
		if (!val) val = 1u;
		vTaskDelay(500u / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}

/* ms */
#define MAX_DELAY 1u

void cons(void *_arg) {
	unsigned int val;
	
	(void)_arg;
	
	while (1) {
		if (xQueueReceive(q, &val, MAX_DELAY)) printf("dequeue %u\n", val);
		else printf("NO DATA ON QUEUE!!!\n");
	}
	
	vTaskDelete(NULL);
}

void app_main(void) {
	BaseType_t ret;
	TaskHandle_t p0;
	TaskHandle_t p1;
	TaskHandle_t c;
	unsigned int i;
	
	q = xQueueCreate(16, sizeof (unsigned int));
	if (!q) {
		printf("cannot create queue\n");
		goto ERR;
	}
	
	ret = xTaskCreate(prod0, "producer0", 4u * configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &p0);
	if (ret != pdPASS) {
		printf("error creating producer0\n");
		goto PROD0_ERR;
	}
	
	ret = xTaskCreate(prod1, "producer1", 4u * configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1u, &p1);
	if (ret != pdPASS) {
		printf("error creating producer1\n");
		goto PROD1_ERR;
	}
	
	ret = xTaskCreate(cons, "consumer", 4u * configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2u, &c);
	if (ret != pdPASS) {
		printf("error creating consumer\n");
		goto CONS_ERR;
	}
	
	for (i = 0u; 1; i++) { /* house keeping */
		vTaskDelay(10000u / portTICK_PERIOD_MS);
	}
	
CONS_ERR:
	vTaskDelete(&p0);
	vTaskDelete(&p1);
	
PROD1_ERR:
	vTaskDelete(&p0);
	
PROD0_ERR:
	vQueueDelete(q);

ERR:
	vTaskDelete(NULL);
}

