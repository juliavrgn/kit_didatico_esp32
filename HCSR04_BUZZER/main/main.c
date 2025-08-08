#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include "esp_timer.h"

#define TRIGGER_GPIO GPIO_NUM_5
#define ECHO_GPIO GPIO_NUM_18
#define BUZZER_GPIO GPIO_NUM_15

#define MAX_DISTANCE_CM 400
#define SOUND_SPEED_CM_PER_US 0.0343f

static const char *TAG = "HC-SR04";

#define MAX_WAIT_TIME_US 30000

void ultrasonic_task(void *pvParameter) {
    // Configura pinos
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << TRIGGER_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1ULL << ECHO_GPIO);
    io_conf.mode = GPIO_MODE_INPUT;
    gpio_config(&io_conf);

    // Configura pino do buzzer como saída
    io_conf.pin_bit_mask = (1ULL << BUZZER_GPIO);
    io_conf.mode = GPIO_MODE_OUTPUT;
    gpio_config(&io_conf);

    while(1) {
        // Envia pulso HIGH de 10us no trigger
        gpio_set_level(TRIGGER_GPIO, 0);
        esp_rom_delay_us(2);
        gpio_set_level(TRIGGER_GPIO, 1);
        esp_rom_delay_us(10);
        gpio_set_level(TRIGGER_GPIO, 0);

        int64_t start_wait;
        uint32_t start_time = 0, end_time = 0;

        // Aguarda pulso HIGH no echo com timeout
        start_wait = esp_timer_get_time();
        while (gpio_get_level(ECHO_GPIO) == 0) {
            if ((esp_timer_get_time() - start_wait) > MAX_WAIT_TIME_US) {
                ESP_LOGW(TAG, "Timeout esperando pulso HIGH no echo");
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        start_time = esp_timer_get_time();

        // Aguarda pulso LOW no echo com timeout
        start_wait = esp_timer_get_time();
        while (gpio_get_level(ECHO_GPIO) == 1) {
            if ((esp_timer_get_time() - start_wait) > MAX_WAIT_TIME_US) {
                ESP_LOGW(TAG, "Timeout esperando pulso LOW no echo");
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        end_time = esp_timer_get_time();

        uint32_t pulse_duration = end_time - start_time;

        float distance_cm = (pulse_duration * SOUND_SPEED_CM_PER_US) / 2.0f;

        if(distance_cm > 0 && distance_cm <= MAX_DISTANCE_CM) {
            ESP_LOGI(TAG, "Distância: %.2f cm", distance_cm);

            // Aciona buzzer se distância < 10cm
            if(distance_cm < 10.0f) {
                gpio_set_level(BUZZER_GPIO, 1); // Liga buzzer
            } else {
                gpio_set_level(BUZZER_GPIO, 0); // Desliga buzzer
            }

        } else {
            ESP_LOGI(TAG, "Distância fora do alcance ou erro: %.2f cm", distance_cm);
            gpio_set_level(BUZZER_GPIO, 0); // Desliga buzzer em erro
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // espera 1 segundo
    }
}

void app_main() {
    xTaskCreate(&ultrasonic_task, "ultrasonic_task", 2048, NULL, 5, NULL);
}
