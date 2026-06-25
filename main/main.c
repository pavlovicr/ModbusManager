/*
 * ESP32-S3 Modbus TCP Master
 * ZLAN gateway → več Modbus RTU slave naprav
 *
 * Arhitektura:
 *   main.c           - vstopna točka, zanka branja
 *   wifi_manager     - WiFi STA inicializacija
 *   modbus_master    - esp-modbus TCP inicializacija in branje
 *   devices/dtsu666  - CHINT DTSU666-H register mapa in podatki
 *
 * Za dodajanje nove naprave:
 *   1. Ustvari devices/nova_naprava.h + .c
 *   2. Dodaj slave ID v app_config.h
 *   3. Spoji deskriptorje v build_combined_descriptor() spodaj
 *   4. Kliči read funkcijo v reading_task()
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "app_config.h"
#include "wifi_manager.h"
#include "modbus_master.h"
#include "dtsu666.h"

// Dodaj sem include za vsako naslednjo napravo:
// #include "devices/sdm120.h"

#define TAG "MAIN"

// ─── Skupna deskriptorska tabela ─────────────────────────────────────────────
// Ob dodajanju naprave: poveži njeno tabelo tukaj in zabeleži cid_offset.

#define DTSU666_CID_OFFSET  0   // DTSU666 zasede CID 0..DTSU_CID_COUNT-1
// #define SDM120_CID_OFFSET (DTSU666_CID_OFFSET + DTSU_CID_COUNT)

static mb_parameter_descriptor_t s_combined_descriptors[64]; // dovolj za ~3 naprave
static uint16_t s_combined_count = 0;

static void build_combined_descriptor(void)
{
    uint16_t count = 0;
    const mb_parameter_descriptor_t *desc;

    // ── DTSU666-H ──
    desc = dtsu666_get_descriptors(&count);
    memcpy(&s_combined_descriptors[s_combined_count], desc,
           count * sizeof(mb_parameter_descriptor_t));
    s_combined_count += count;

    // ── Naslednja naprava (primer) ──
    // desc = sdm120_get_descriptors(&count);
    // // Popravi CID vrednosti za offset
    // for (uint16_t i = 0; i < count; i++) {
    //     s_combined_descriptors[s_combined_count + i] = desc[i];
    //     s_combined_descriptors[s_combined_count + i].cid += SDM120_CID_OFFSET;
    // }
    // s_combined_count += count;

    ESP_LOGI(TAG, "Skupna deskriptorska tabela: %d parametrov", s_combined_count);
}

// ─── Bralna naloga ────────────────────────────────────────────────────────────

static void reading_task(void *pvParameters)
{
    dtsu666_data_t dtsu_data = {0};

    while (1) {
        ESP_LOGI(TAG, "── Cikel branja ──────────────────────────────");

        // Beri DTSU666-H
        esp_err_t err = dtsu666_read_all(DTSU666_CID_OFFSET, &dtsu_data);
        if (err == ESP_OK) {
            dtsu666_print(&dtsu_data);
        } else {
            ESP_LOGE(TAG, "DTSU666-H: branje neuspešno");
        }

        // Beri naslednjo napravo:
        // sdm120_read_all(SDM120_CID_OFFSET, &sdm_data);
        // sdm120_print(&sdm_data);

        vTaskDelay(pdMS_TO_TICKS(MODBUS_READ_PERIOD_MS));
    }
}

// ─── Vstopna točka ────────────────────────────────────────────────────────────

void app_main(void)
{
    ESP_LOGI(TAG, "═══════════════════════════════════════════");
    ESP_LOGI(TAG, "  ESP32-S3 Modbus TCP Master  (esp-modbus) ");
    ESP_LOGI(TAG, "═══════════════════════════════════════════");

    // 1. WiFi
    ESP_ERROR_CHECK(wifi_manager_init());
    vTaskDelay(pdMS_TO_TICKS(1000)); // Kratek premor za mrežno stabilizacijo

    // 2. Sestavi skupno deskriptorsko tabelo iz vseh naprav
    build_combined_descriptor();

    // 3. Inicializiraj Modbus TCP Master
    ESP_ERROR_CHECK(modbus_master_init(s_combined_descriptors, s_combined_count));

    // 4. Zaženi branje v FreeRTOS nalogi
    xTaskCreate(reading_task, "modbus_read", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "Sistem zagnan. Branje vsakih %d ms.", MODBUS_READ_PERIOD_MS);
}
