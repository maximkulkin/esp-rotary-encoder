#include <stdio.h>
#include <stdlib.h>
#include <esp/uart.h>
#include <FreeRTOS.h>
#include <task.h>

#include <rotary_encoder.h>

#ifndef A_GPIO
#error A_GPIO just be defined
#endif
#ifndef B_GPIO
#error B_GPIO just be defined
#endif

void idle_task(void* arg) {
    while (true) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}


void rotary_encoder_callback(uint8_t position, void* context) {
    printf("position: %d\n", position);
}


void user_init(void) {
    uart_set_baud(0, 115200);

    printf("Rotary encoder example\n");

    int r;
    r = rotary_encoder_create(A_GPIO, B_GPIO, rotary_encoder_callback, NULL);
    if (!r) {
        printf("Failed to initialize rotary encoder\n");
    }

    xTaskCreate(idle_task, "Idle task", 256, NULL, tskIDLE_PRIORITY, NULL);
}
