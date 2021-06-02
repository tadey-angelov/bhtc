#include <stdio.h>

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/message_buffer.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

typedef struct our_struct_t {
	int a, b;
} our_struct_t;

void prod(void *_arg) {
	MessageBufferHandle_t *mb = (MessageBufferHandle_t *)_arg;
	our_struct_t s = {
		.a = 0,
		.b = 0
	};
	size_t len;
	
	while (1) {
		printf("sending %d %d\n", s.a, s.b);
		len = xMessageBufferSend(*mb, (void *)&s, sizeof (s), portMAX_DELAY);
		printf("sent %u %d %d\n", len, s.a, s.b);
		s.a++;
		s.b += 2;
		vTaskDelay(1000u / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}

void cons(void *_arg) {
	MessageBufferHandle_t *mb = (MessageBufferHandle_t *)_arg;
	our_struct_t s;
	size_t len;
	
	while (1) {
		len = xMessageBufferReceive(*mb, &s, sizeof (s), portMAX_DELAY);
		printf("got %u %d %d %d\n", len, s.a, s.b, s.a + s.b);
	}
	
	vTaskDelete(NULL);
}

void app_main(void) {
	static TaskHandle_t producer_hnd, consumer_hnd;
	static MessageBufferHandle_t mb_hnd;
	static StaticTask_t producer, consumer;
	static StaticMessageBuffer_t mb;
	static StackType_t producer_stack[4u * configMINIMAL_STACK_SIZE];
	static StackType_t consumer_stack[4u * configMINIMAL_STACK_SIZE];
	static our_struct_t buf[2];
	int i;
	
	mb_hnd = xMessageBufferCreateStatic(sizeof (buf), (uint8_t *)&buf, &mb);
	
	producer_hnd = xTaskCreateStatic(
			prod,
			"producer_task",
			4u * configMINIMAL_STACK_SIZE,
			&mb_hnd,
			tskIDLE_PRIORITY,
			producer_stack,
			&producer
	);
	
	consumer_hnd = xTaskCreateStatic(
			cons,
			"consumer_task",
			4u * configMINIMAL_STACK_SIZE,
			&mb_hnd,
			tskIDLE_PRIORITY,
			consumer_stack,
			&consumer
	);
	
	(void)producer_hnd;
	(void)consumer_hnd;
	
	for (i = 0u; 1; i++) { /* house keeping */
		printf("house keeping...\n");
		vTaskDelay(10000u / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}

