#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define PIN_SENSOR_UMIDADE 26
#define PIN_LED_INDICADOR  15

#define LIMITE_UMIDADE 2000

QueueHandle_t filaUmidade;

typedef struct {
    uint16_t valor;
} dado_umidade_t;

void tarefaLeituraSensor(void *params);
void tarefaControleIrrigacao(void *params);

int main() {
    stdio_init_all();

    adc_init();
    adc_gpio_init(PIN_SENSOR_UMIDADE);
    adc_select_input(0);

    gpio_init(PIN_LED_INDICADOR);
    gpio_set_dir(PIN_LED_INDICADOR, GPIO_OUT);
    gpio_put(PIN_LED_INDICADOR, 0);

    filaUmidade = xQueueCreate(5, sizeof(dado_umidade_t));

    xTaskCreate(tarefaLeituraSensor, "LeituraSensor", 256, NULL, 1, NULL);
    xTaskCreate(tarefaControleIrrigacao, "ControleIrrigacao", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true) {
    }
}

void tarefaLeituraSensor(void *params) {
    dado_umidade_t dado;

    while (1) {
        dado.valor = adc_read();
        xQueueSend(filaUmidade, &dado, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void tarefaControleIrrigacao(void *params) {
    dado_umidade_t dado;

    while (1) {
        if (xQueueReceive(filaUmidade, &dado, portMAX_DELAY)) {
            if (dado.valor < LIMITE_UMIDADE) {
                gpio_put(PIN_LED_INDICADOR, 1);
            } else {
                gpio_put(PIN_LED_INDICADOR, 0);
            }
        }
    }
}
