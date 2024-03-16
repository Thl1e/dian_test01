#include <stdio.h>
#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <inttypes.h>
#include "sdkconfig.h"

static char msg[20]="hellow world\n";

void taskBTsend(void *args){
    int a=uart_write_bytes(UART_NUM_1,&msg,20);
    printf("%d\n",a);
    vTaskDelay(200);
    printf("Restarting\n");
    vTaskDelay(500);

}
void app_main(void)
{
    QueueHandle_t eventQueue;

   uart_config_t uart0={
        .baud_rate= 9600,
        .data_bits= UART_DATA_8_BITS,
        .flow_ctrl= UART_HW_FLOWCTRL_DISABLE,
        .parity= UART_PARITY_DISABLE,
        .stop_bits= UART_STOP_BITS_1,
        .source_clk= UART_SCLK_DEFAULT,
   };

   uart_param_config(UART_NUM_1,&uart0);

   uart_driver_install(
        UART_NUM_1,
        1024,
        1024,
        16,
        &eventQueue,
        0
   );

    uart_set_pin(
        UART_NUM_1,
        1,
        2,
        3,
        4
        
    );
    xTaskCreate(taskBTsend,"taskBTsent",4096,NULL,4,NULL);

}
