#include <stdio.h>

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

static SemaphoreHandle_t sem_hnd, mtx_hnd;
static StaticSemaphore_t sem, mtx;

void sleep_task(void *_ms) {
	uint32_t ms = (uint32_t)_ms;
	int i;
	
	for (i = 0; i < 48; i++) {
		xSemaphoreTake(mtx_hnd, portMAX_DELAY);
		printf("sleep_task(%u) sleeping...\n", ms);
		xSemaphoreGive(mtx_hnd);
		vTaskDelay(ms / portTICK_PERIOD_MS);
	}
	
	printf("sleep_task(%u) terminating...\n", ms);	
	vTaskDelete(NULL);
}

void app_main(void) {
	TaskHandle_t task_hnd;
	static StaticTask_t task;
	static StackType_t stack[2u * configMINIMAL_STACK_SIZE];
	int i;
	
	task_hnd = xTaskCreateStatic(
			sleep_task,
			"static_sleep_task",
			2u * configMINIMAL_STACK_SIZE,
			(void *)500ul,
			tskIDLE_PRIORITY,
			stack,
			&task
	);
	
#if 0
	mtx_hnd = xSemaphoreCreateBinaryStatic(&mtx);
	xSemaphoreGive(mtx_hnd);
#elif 0
	mtx_hnd = xSemaphoreCreateCountingStatic(1u, 0u, &mtx);
	xSemaphoreGive(mtx_hnd);
#else
	mtx_hnd = xSemaphoreCreateCountingStatic(1u, 1u, &mtx);
	(void)mtx_hnd;
#endif
	sem_hnd = xSemaphoreCreateCountingStatic(16u, 0u, &sem);
	(void)sem_hnd;
	
	(void)task_hnd;
	
	for (i = 0; i < 60; i++) {
		xSemaphoreTake(mtx_hnd, portMAX_DELAY);
		printf("Hello world from FreeRTOS! %u\n", i);
		xSemaphoreGive(mtx_hnd);
		vTaskDelay(1000u / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}

