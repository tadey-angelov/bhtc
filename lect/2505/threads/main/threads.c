#include <stdio.h>

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

void sleep_task(void *_ms) {
	uint32_t ms = (uint32_t)_ms;
	
	while (1) {
		printf("sleep_task(%u) sleeping...\n", ms);
		vTaskDelay(ms / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}

void app_main(void) {
	BaseType_t ret;
	TaskHandle_t hnd[10];
	unsigned int i, j;
	
	for (i = 0u; i < sizeof (hnd) / sizeof (hnd[0]); i++) {
		ret = xTaskCreate(sleep_task, "sleeper", 2u * configMINIMAL_STACK_SIZE, (void *)(1000u * (i + 1u)), tskIDLE_PRIORITY, &hnd[i]);
		if (ret != pdPASS) {
			printf("error creating sleeper %u\n", i);
			for (j = 0; j < i; j++)
				vTaskDelete(hnd[j]);
			goto ERR;
		}
	}
	
	for (i = 0u; 1; i++) {
		printf("Hello world from FreeRTOS! %u\n", i);
		vTaskDelay(1000u / portTICK_PERIOD_MS);
	}
	
ERR:
	vTaskDelete(NULL);
}

