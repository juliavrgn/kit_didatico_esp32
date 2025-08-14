#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_err.h"

#include "keypad.h"     
#include <ssd1306.h>    

// Define os 8 pinos: primeiros 4 são linhas (rows), últimos 4 colunas (cols)
static gpio_num_t keypad_pins[8] = {
    GPIO_NUM_15, GPIO_NUM_4, GPIO_NUM_16, GPIO_NUM_17,    // linhas
    GPIO_NUM_18, GPIO_NUM_19, GPIO_NUM_32, GPIO_NUM_33    // colunas
};

void app_main(void) {
    // Inicializa o display SSD1306 (deve ter sua própria implementação)
    init_ssd1306();

    // Mostra mensagem inicial no display
    ssd1306_clear();
    ssd1306_print_str(0, 0, "Aperte a tecla:", false);
    ssd1306_display();

    // Inicializa o teclado matricial com a lista única de 8 pinos
    esp_err_t ret = keypad_initalize(keypad_pins);
    if (ret != ESP_OK) {
        printf("Erro ao inicializar o teclado matricial\n");
        return; // ou tratar erro adequadamente
    }

    char prev_key = '\0'; // caracter para controle de tecla repetida

    while (1) {
        // Lê tecla pressionada
        char key = keypad_getkey();

        if (key != '\0' && key != prev_key) {  // nova tecla pressionada (não '\0' e diferente da anterior)
            // Limpa display e escreve a mensagem + tecla pressionada
            ssd1306_clear();
            ssd1306_print_str(0, 0, "Aperte a tecla:", false);
            
            
            char key_str[2] = {key, '\0'};
            ssd1306_print_str(40, 20, "Tecla:", false);
            ssd1306_print_str(80, 20, key_str, false);
            
            ssd1306_display();

            prev_key = key;
        } else if (key == '\0') {
            // Nenhuma tecla pressionada, reseta o controle para nova detecção
            prev_key = '\0';
        }

        vTaskDelay(pdMS_TO_TICKS(100));  // polling suave a cada 100 ms
    }
}
