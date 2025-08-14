#include <stdio.h>              
#include <string.h>             
#include "dht11.h"              
#include "freertos/FreeRTOS.h"  
#include <ssd1306.h>            

// Pino GPIO conectado aos dados do DHT11
#define CONFIG_DHT11_PIN GPIO_NUM_4

// Tempo de conexão para o DHT11 em segundos
#define CONFIG_CONNECTION_TIMEOUT 5


void app_main() {
  
  // Configuração da estrutura do DHT11
  dht11_t dht11_sensor;
  dht11_sensor.dht11_pin = CONFIG_DHT11_PIN;

  // Inicializa o display
  init_ssd1306();

  // Strings para temperatura e umidade a serem exibidas no display
  char temp_str[20];
  char umid_str[25];

  // Loop infinito para leitura de dados
  while (1) {
    // Variáveis para salvar as leituras anteriores de dados
    static uint8_t prev_temp = 0xFF; // estado inicial
    static uint8_t prev_umi  = 0xFF;

    // Leitura do sensor. Retorna 0 quando bem sucedido
    if (!dht11_read(&dht11_sensor, CONFIG_CONNECTION_TIMEOUT)) {
      // Cast sensor readings to 8-bit unsigned integers
      uint8_t curr_temp = (uint8_t)dht11_sensor.temperature;
      uint8_t curr_umi  = (uint8_t)dht11_sensor.humidity;

      // Atualiza o display quando os valores mudam
      if (curr_temp != prev_temp || curr_umi != prev_umi) {
        // limpa o display
        ssd1306_clear();

        // Formatação das strings de temperatura e umidade
        sprintf(temp_str,     "Temp: %dC", curr_temp);
        sprintf(umid_str, "Umidade: %d%%", curr_umi);

        // Print das strings no display
        ssd1306_print_str(25, 18, temp_str, false);     
        ssd1306_print_str(8,  28, umid_str, false); 

        // Buffer do display 
        ssd1306_display();

        // Salvando os valores atuais de temperatura e umidade para comparação
        prev_temp = curr_temp;
        prev_umi  = curr_umi;
      }
    }

    // Delay entre leituras para melhor funcionamento
    vTaskDelay(2000 / portTICK_PERIOD_MS); // 2 segundos
  }
}