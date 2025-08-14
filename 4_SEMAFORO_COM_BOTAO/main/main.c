#include <stdio.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"

// Pinos dos LEDs
#define GREEN_LED_PIN   GPIO_NUM_2
#define YELLOW_LED_PIN  GPIO_NUM_4
#define RED_LED_PIN     GPIO_NUM_5

// Pino do botão
#define BUTTON_GPIO     GPIO_NUM_15

// Durações (em ms)
#define GREEN_TIME_MS   60000
#define YELLOW_TIME_MS  3000
#define RED_TIME_MS     60000

// Flag global
volatile int button_pressed_flag = 0;

// Detecta pressão do botão
void check_button_override(void) {
    static int last_state = 1; // pull-up: 1 = solto
    int state = gpio_get_level(BUTTON_GPIO);

    if (state == 0 && last_state == 1) { // Pressionado
        printf("Botão pressionado! Sequência: amarelo → vermelho.\n");
        button_pressed_flag = 1;
    }
    last_state = state;
}

// Executa amarelo e vermelho por override
void yellow_then_red_override(void) {
    // Amarelo
    gpio_set_level(GREEN_LED_PIN, 0);
    gpio_set_level(YELLOW_LED_PIN, 1);
    gpio_set_level(RED_LED_PIN, 0);
    printf("Amarelo (override) por %d ms\n", YELLOW_TIME_MS);
    vTaskDelay(pdMS_TO_TICKS(YELLOW_TIME_MS));

    // Vermelho
    gpio_set_level(GREEN_LED_PIN, 0);
    gpio_set_level(YELLOW_LED_PIN, 0);
    gpio_set_level(RED_LED_PIN, 1);
    printf("Vermelho (override) por %d ms\n", RED_TIME_MS);
    vTaskDelay(pdMS_TO_TICKS(RED_TIME_MS));

    // Limpa flag
    button_pressed_flag = 0;
    printf("Override finalizado. Retornando ao verde.\n");
}

void traffic_signal_task(void *pvParameter) {
    while (1) {
        // Verde
        printf("LED Verde ON\n");
        gpio_set_level(GREEN_LED_PIN, 1);
        gpio_set_level(YELLOW_LED_PIN, 0);
        gpio_set_level(RED_LED_PIN, 0);
        for (int i = 0; i < GREEN_TIME_MS / 100; i++) {
            check_button_override();
            if (button_pressed_flag) {
                yellow_then_red_override();
                break; // Sai do verde e reinicia no verde
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        if (button_pressed_flag) continue; // Força retorno ao início (verde)

        // Amarelo
        printf("LED Amarelo ON\n");
        gpio_set_level(GREEN_LED_PIN, 0);
        gpio_set_level(YELLOW_LED_PIN, 1);
        gpio_set_level(RED_LED_PIN, 0);
        for (int i = 0; i < YELLOW_TIME_MS / 100; i++) {
            check_button_override();
            if (button_pressed_flag) {
                yellow_then_red_override();
                break; // Sai do amarelo e reinicia no verde
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        if (button_pressed_flag) continue;

        // Vermelho
        printf("LED Vermelho ON\n");
        gpio_set_level(GREEN_LED_PIN, 0);
        gpio_set_level(YELLOW_LED_PIN, 0);
        gpio_set_level(RED_LED_PIN, 1);
        for (int i = 0; i < RED_TIME_MS / 100; i++) {
            check_button_override();
            if (button_pressed_flag) {
                yellow_then_red_override();
                break; // Sai do vermelho e reinicia no verde
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void app_main(void) {
    // Configuração dos LEDs
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << GREEN_LED_PIN) | (1ULL << YELLOW_LED_PIN) | (1ULL << RED_LED_PIN),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE
    };
    gpio_config(&io_conf);

    // Configuração do botão como entrada com pull-up
    gpio_config_t btn_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    gpio_config(&btn_conf);

    // Criação da task do semáforo
    xTaskCreate(traffic_signal_task, "traffic_signal_task", 4096, NULL, 5, NULL);
}