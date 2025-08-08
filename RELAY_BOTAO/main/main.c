#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_rom_gpio.h"


#define RELAY_GPIO GPIO_NUM_23    // GPIO para o relé 
#define BUTTON_GPIO GPIO_NUM_5    // GPIO para o botão 

static int relay_state = 0;       // Estado atual do relé (0 = desligado, 1 = ligado)

void relay_button_task(void *pvParameter) {
    // Configura GPIO do relé como saída
    esp_rom_gpio_pad_select_gpio(RELAY_GPIO);
    gpio_set_direction(RELAY_GPIO, GPIO_MODE_OUTPUT);

    // Configura GPIO do botão como entrada com pull-up interno
    esp_rom_gpio_pad_select_gpio(BUTTON_GPIO);
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);

    int button_last_state = 1;    // botão pull-up = 1 quando não pressionado
    int button_current_state;

    while (1) {
        button_current_state = gpio_get_level(BUTTON_GPIO);

        // Detecta borda de descida (botão pressionado)
        if (button_last_state == 1 && button_current_state == 0) {
            // Aguarda debounce simples
            vTaskDelay(pdMS_TO_TICKS(50));
            // Confirma botão ainda pressionado
            if (gpio_get_level(BUTTON_GPIO) == 0) {
                // Inverte estado do relé
                relay_state = !relay_state;
                gpio_set_level(RELAY_GPIO, relay_state);
                printf("Botão pressionado! Relé %s\n", relay_state ? "LIGADO" : "DESLIGADO");

                // Aguarda o botão ser solto para evitar múltiplas detecções
                while (gpio_get_level(BUTTON_GPIO) == 0) {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
            }
        }
        button_last_state = button_current_state;

        vTaskDelay(pdMS_TO_TICKS(10)); // Pequena pausa para debounce e economia de CPU
    }
}

void app_main(void) {
    xTaskCreate(&relay_button_task, "relay_button_task", 2048, NULL, 5, NULL);
}
