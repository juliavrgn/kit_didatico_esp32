#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/adc.h>
#include <ssd1306.h>   // Supondo que você já tenha o driver SSD1306 configurado e inicializado

// Pino ADC conectado ao LDR (exemplo: GPIO34 que é ADC1_CH6)
#define LDR_ADC_CHANNEL ADC1_CHANNEL_6

// Função para inicializar o ADC
void adc_init(void) {
    adc1_config_width(ADC_WIDTH_BIT_12);        // Resolução 12 bits
    adc1_config_channel_atten(LDR_ADC_CHANNEL, ADC_ATTEN_DB_11); // Atenuação para faixa maior
}

void app_main() {
    // Inicializa ADC para ler LDR
    adc_init();

    // Inicializa display SSD1306
    init_ssd1306();

    char ldr_str[30];
    
    while (1) {
        // Leitura do ADC do LDR
        int ldr_value = adc1_get_raw(LDR_ADC_CHANNEL);

        // Limpa o display antes de atualizar
        ssd1306_clear();

        // Formata valor lido para string
        sprintf(ldr_str, "LDR: %d", ldr_value);

        // Exibe a string na posição desejada do display
        ssd1306_print_str(10, 20, ldr_str, false);

        // Atualiza o display para mostrar a escrita
        ssd1306_display();

        // Espera 2 segundos antes da próxima leitura
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
