#include <stdio.h>

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

void app_main(void) {
	int i;
	
	for (i = 0; 1; i++) {
		printf("Hello world from FreeRTOS! %d\n", i);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}

