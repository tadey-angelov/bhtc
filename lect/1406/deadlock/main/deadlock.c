#include <stdio.h>

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

static SemaphoreHandle_t mutex_x_hnd, mutex_y_hnd;
static StaticSemaphore_t mutex_x, mutex_y;

static StaticTask_t taskx;
static StackType_t taskx_stack[2u * configMINIMAL_STACK_SIZE];
static TaskHandle_t taskx_hnd;

static StaticTask_t tasky;
static StackType_t tasky_stack[2u * configMINIMAL_STACK_SIZE];
static TaskHandle_t tasky_hnd;

void tasky_function(void *arg);

void taskx_function(void *arg) {
	int created = 0;
	
	while (1) {
		xSemaphoreTake(mutex_x_hnd, portMAX_DELAY);
		printf("taskx\n");
		
		if (!created) {
			tasky_hnd = xTaskCreateStatic(
					tasky_function,
					"tasky",
					2u * configMINIMAL_STACK_SIZE,
					NULL,
					tskIDLE_PRIORITY + 2u,
					tasky_stack,
					&tasky
			);
			created = 1;
		}
		
		printf("taskx waiting on mutex y\n");
		xSemaphoreTake(mutex_y_hnd, portMAX_DELAY);
/* XXX do nothing */
		xSemaphoreGive(mutex_y_hnd);		
		xSemaphoreGive(mutex_x_hnd);
		
		vTaskDelay(1u);
	}
	
	vTaskDelete(NULL);
}

void tasky_function(void *arg) {
	while (1) {
		xSemaphoreTake(mutex_y_hnd, portMAX_DELAY);
		printf("tasky\n");
				
		printf("tasky waiting on mutex x\n");
		xSemaphoreTake(mutex_x_hnd, portMAX_DELAY);
/* XXX do nothing */
		xSemaphoreGive(mutex_x_hnd);		
		xSemaphoreGive(mutex_y_hnd);
		
		vTaskDelay(1u);
	}
	
	vTaskDelete(NULL);
}

void app_main(void) {	
	mutex_x_hnd = xSemaphoreCreateBinaryStatic(&mutex_x);
	xSemaphoreGive(mutex_x_hnd);
	mutex_y_hnd = xSemaphoreCreateBinaryStatic(&mutex_y);
	xSemaphoreGive(mutex_y_hnd);
	
	vTaskDelay(10000u / portTICK_PERIOD_MS);
		
	taskx_hnd = xTaskCreateStatic(
			taskx_function,
			"taskx",
			2u * configMINIMAL_STACK_SIZE,
			NULL,
			tskIDLE_PRIORITY + 1u,
			taskx_stack,
			&taskx
	);
	(void)taskx_hnd;
	
	while (1) {
		printf("housekeeping...\n");
		vTaskDelay(1000u / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}

