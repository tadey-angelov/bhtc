#include <stdio.h>

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#define RECURSIVE

static SemaphoreHandle_t mutex_hnd;
static StaticSemaphore_t mutex;

static StaticTask_t task;
static StackType_t task_stack[4u * configMINIMAL_STACK_SIZE];
static TaskHandle_t task_hnd;

typedef int (*callback_t)(int);

static int i;

int deadlock(callback_t callback) {
	i++;
	if (i > 4) return -1;
	else return callback(1);
}

static int test_deadlock(int i) {
	int t;

#ifdef RECURSIVE	
	xSemaphoreTakeRecursive(mutex_hnd, portMAX_DELAY);
#else
	xSemaphoreTake(mutex_hnd, portMAX_DELAY);
#endif
	
	printf("test_deadlock will deadlock\n");
	t = deadlock(test_deadlock);

#ifdef RECURSIVE
	xSemaphoreGiveRecursive(mutex_hnd);
#else
	xSemaphoreGive(mutex_hnd);
#endif
	
	return t;
}

void task_function(void *arg) {
	while (1) {
		i = 0;
		printf("task\n");
		printf("%d\n", test_deadlock(0));
		vTaskDelay(1000u / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}

void app_main(void) {
#ifdef RECURSIVE
	mutex_hnd = xSemaphoreCreateRecursiveMutexStatic(&mutex);
	xSemaphoreGiveRecursive(mutex_hnd);
#else
	mutex_hnd = xSemaphoreCreateMutexStatic(&mutex);
	xSemaphoreGive(mutex_hnd);
#endif
	
	vTaskDelay(10000u / portTICK_PERIOD_MS);
		
	task_hnd = xTaskCreateStatic(
			task_function,
			"task",
			4u * configMINIMAL_STACK_SIZE,
			NULL,
			tskIDLE_PRIORITY + 1u,
			task_stack,
			&task
	);
	(void)task_hnd;
	
	while (1) {
		printf("housekeeping...\n");
		vTaskDelay(1000u / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}

