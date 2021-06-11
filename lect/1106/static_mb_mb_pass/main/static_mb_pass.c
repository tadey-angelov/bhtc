#include <stdio.h>

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/message_buffer.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

typedef enum our_request_t {
	GETDATA
} our_request_t;

typedef enum out_datatype_t {
	STRUCT,
	INT
} our_datatype_t;

typedef struct our_struct_t {
	int a, b;
} our_struct_t;

typedef union out_data_t {
	int int_t;
	our_struct_t struct_t;
} our_data_t;

typedef struct our_msg_request_t {
	our_request_t request;
	our_datatype_t type;
	MessageBufferHandle_t *response_mb; /* mailbox of type our_response_t */
} our_msg_request_t;

typedef struct our_response_t {
	our_datatype_t type;
	our_data_t data;
} our_response_t;

void driver_task_function(void *_arg) {
	MessageBufferHandle_t *driver_msg_hnd = (MessageBufferHandle_t *)_arg;
	our_msg_request_t request;
	our_struct_t struct_t = {
		.a = 0,
		.b = 0
	};
	int int_t = 0;
	our_response_t response;
	size_t len;
	
	while (1) {
		len = xMessageBufferReceive(*driver_msg_hnd, &request, sizeof (request), portMAX_DELAY);
		switch (request.request) {
		case GETDATA:
			switch (request.type) {
			case STRUCT:
				printf("sending struct %d %d\n", struct_t.a, struct_t.b);
				response.type = STRUCT;
				response.data.struct_t = struct_t;
				len = xMessageBufferSend(*(request.response_mb), (void *)&response, sizeof (response), portMAX_DELAY);
				printf("sent struct %u %d %d\n", len, struct_t.a, struct_t.b);
				struct_t.a++;
				struct_t.b += 2;
				break;
			case INT:
				printf("sending int %d\n", int_t);
				response.type = INT;
				response.data.int_t = int_t;
				len = xMessageBufferSend(*(request.response_mb), (void *)&response, sizeof (response), portMAX_DELAY);
				printf("sent int %u %d\n", len, int_t);
				int_t += 5;
				break;			
			default:
				printf("unknow data request recv'd...\n");
				continue;
			}
			break;
		default:
			printf("unknow request recv'd...\n");
			continue;
		}
	}
	
	vTaskDelete(NULL);
}

void client_task_function_struct(void *_arg) {
	MessageBufferHandle_t *driver_msg_hnd = (MessageBufferHandle_t *)_arg;
	static MessageBufferHandle_t mb_response_hnd;
	static StaticMessageBuffer_t mb_response_t;
	static uint8_t buf[sizeof (our_response_t) + 2u * sizeof (size_t)];
	our_response_t response;
	size_t len;
	
	mb_response_hnd = xMessageBufferCreateStatic(sizeof (buf), buf, &mb_response_t);
	
	while (1) {
		our_msg_request_t request = {
			.request = GETDATA,
			.type = STRUCT,
			.response_mb = &mb_response_hnd
		};
		len = xMessageBufferSend(*driver_msg_hnd, &request, sizeof (request), portMAX_DELAY);
		printf("\trequest sent %u\n", len);
		len = xMessageBufferReceive(mb_response_hnd, &response, sizeof (response), portMAX_DELAY);
		printf("\tgot %u %d %d\n", len, response.data.struct_t.a, response.data.struct_t.b);
		vTaskDelay(1000u / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}

void client_task_function_int(void *_arg) {
	MessageBufferHandle_t *driver_msg_hnd = (MessageBufferHandle_t *)_arg;
	static MessageBufferHandle_t mb_response_hnd;
	static StaticMessageBuffer_t mb_response_t;
	static uint8_t buf[sizeof (our_response_t) + 2u * sizeof (size_t)];
	our_response_t response;
	size_t len;
	
	mb_response_hnd = xMessageBufferCreateStatic(sizeof (buf), buf, &mb_response_t);
	
	while (1) {
		our_msg_request_t request = {
			.request = GETDATA,
			.type = INT,
			.response_mb = &mb_response_hnd
		};
		len = xMessageBufferSend(*driver_msg_hnd, &request, sizeof (request), portMAX_DELAY);
		printf("\t\trequest sent %u\n", len);
		len = xMessageBufferReceive(mb_response_hnd, &response, sizeof (response), portMAX_DELAY);
		printf("\t\tgot %u %d\n", len, response.data.int_t);
		vTaskDelay(1000u / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}

void app_main(void) {
	static StaticTask_t driver_task;
	static StackType_t driver_stack[4u * configMINIMAL_STACK_SIZE];
	static TaskHandle_t driver_hnd;
	
	static StaticTask_t client_task_struct;
	static StackType_t client_stack_struct[4u * configMINIMAL_STACK_SIZE];
	static TaskHandle_t client_hnd_struct;
	
	static StaticTask_t client_task_int;
	static StackType_t client_stack_int[4u * configMINIMAL_STACK_SIZE];
	static TaskHandle_t client_hnd_int;
		
	static StaticMessageBuffer_t driver_msg_buf;
	static uint8_t driver_buf[sizeof (our_msg_request_t) + 2u * sizeof (size_t)];
	static MessageBufferHandle_t driver_msg_hnd;
	
	unsigned i;
	
	driver_msg_hnd = xMessageBufferCreateStatic(sizeof (driver_buf), driver_buf, &driver_msg_buf);
	
	vTaskDelay(10000u / portTICK_PERIOD_MS);
	printf("starting tasks...\n");
	
	driver_hnd = xTaskCreateStatic(
			driver_task_function,
			"driver_task",
			4u * configMINIMAL_STACK_SIZE,
			&driver_msg_hnd,
			tskIDLE_PRIORITY + 16u,
			driver_stack,
			&driver_task
	);
	(void)driver_hnd;
	
	client_hnd_struct = xTaskCreateStatic(
			client_task_function_struct,
			"client_task_struct",
			4u * configMINIMAL_STACK_SIZE,
			&driver_msg_hnd,
			tskIDLE_PRIORITY + 8u,
			client_stack_struct,
			&client_task_struct
	);
	(void)client_hnd_struct;
	
	client_hnd_int = xTaskCreateStatic(
			client_task_function_int,
			"client_task_int",
			4u * configMINIMAL_STACK_SIZE,
			&driver_msg_hnd,
			tskIDLE_PRIORITY + 12u,
			client_stack_int,
			&client_task_int
	);
	(void)client_hnd_int;
	
	for (i = 0u; 1; i++) { /* house keeping */
		printf("\t\t\thouse keeping %u...\n", i);
		vTaskDelay(10000u / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}

