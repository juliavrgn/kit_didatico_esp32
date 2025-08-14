#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/adc.h>
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <esp_log.h>
#include "driver/sdmmc_host.h"
#include "esp_timer.h"


static const char *TAG = "LDR_SD";

#define ADC_CHANNEL ADC1_CHANNEL_6  // GPIO34, ADC1_CH6 (ajuste conforme seu pino)
#define ADC_ATTEN ADC_ATTEN_DB_11   // Atenuação para maior faixa (0-3.6V)
#define MOUNT_POINT "/sdcard"

void app_main(void)
{
    esp_err_t ret;

    // Configura ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);

    // Configuração SD card
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false, // NÃO formata SD se falhar montagem
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_card_t* card;
    ESP_LOGI(TAG, "Mounting SD card...");
    ret = esp_vfs_fat_sdmmc_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SD card (%s)", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "SD card mounted successfully");

    while (1) {
        // Leitura ADC do LDR
        int adc_reading = adc1_get_raw(ADC_CHANNEL);

        // Abrir arquivo para append (adicionar ao final)
        FILE* f = fopen(MOUNT_POINT"/ldr_data.txt", "a");
        if (f == NULL) {
            ESP_LOGE(TAG, "Falha ao abrir arquivo para escrita");
        } else {
            // Escrever dados e timestamp (tempo desde boot em ms)
            int64_t now = esp_timer_get_time() / 1000; // tempo em ms
            fprintf(f, "%lld ms: %d\n", now, adc_reading);
            fclose(f);
            ESP_LOGI(TAG, "Dado salvo: %lld ms - %d", now, adc_reading);
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // aguarda 1 segundo entre leituras
    }

  
}
