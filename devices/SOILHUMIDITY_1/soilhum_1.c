/**
 * @file soilhum_1.c
 * @brief Senzor vlage in temperature v zemlji - Modbus RTU master gonilnik
 * 
 * Senzor vrača dve 16-bitni integer vrednosti (dejansko vrednost * 10):
 *   0x0000 - Relativna vlaga (0-100%) v 0.1% korakih (npr. 256 = 25.6%)
 *   0x0001 - Temperatura v °C (npr. 235 = 23.5°C, predznak je lahko negativen)
 * 
 * @author Rados
 * @date 2026
 */

#include "soilhum_1.h"

#include <string.h>
#include "esp_log.h"
#include "modbus_master.h"
#include "app_config.h"

// ─── Oznaka za beleženje ──────────────────────────────────────────────────────
#define TAG "SOIL"

// ═══════════════════════════════════════════════════════════════════════════════
// REGISTRSKA PRESLIKAVA - SOIL senzor
// ═══════════════════════════════════════════════════════════════════════════════
//
// Vsi registri so 16-bitni integerji (uint16_t), samo za branje.
// Modbus funkcija: 03 (Beri držalne registre)
//
// Tabela registrov:
//
//  Register | Naziv        | Enota  | Opis
//  ---------|--------------|--------|------------------------------------
//  0x0000   | Vlaga        | %      | 0-100%, vrednost * 10
//  0x0001   | Temperatura  | °C     | Temperatura zemlje, vrednost * 10
//
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Deskriptorska tabela za Modbus master ────────────────────────────────

static const mb_parameter_descriptor_t s_soil_descriptors[] = {
    { SOIL_CID_MOISTURE,    "Vlaga",        "%",   SLAVE_ID_SOIL, MB_PARAM_HOLDING, 0x0000, 1, 0, PARAM_TYPE_U16, PARAM_SIZE_U16, {.opt1=0, .opt2=1000, .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { SOIL_CID_TEMPERATURE, "Temperatura",  "°C",  SLAVE_ID_SOIL, MB_PARAM_HOLDING, 0x0001, 1, 0, PARAM_TYPE_U16, PARAM_SIZE_U16, {.opt1=-400, .opt2=800, .opt3=1}, PAR_PERMS_READ_TRIGGER },
};

// Število parametrov v tabeli
static const uint16_t s_descriptor_count =
    sizeof(s_soil_descriptors) / sizeof(s_soil_descriptors[0]);

// ═══════════════════════════════════════════════════════════════════════════════
// JAVNE FUNKCIJE
// ═══════════════════════════════════════════════════════════════════════════════

const mb_parameter_descriptor_t *soil_get_descriptors(uint16_t *out_count)
{
    if (out_count) {
        *out_count = s_descriptor_count;
    }
    return s_soil_descriptors;
}

/**
 * @brief Beri vse parametre s SOIL senzorja in jih shrani v strukturo
 * 
 * Prebere dva 16-bitna registra (vlago in temperaturo) ter ju pretvori
 * v realne vrednosti z deljenjem z 10.
 * 
 * @param cid_offset Začetni CID odmik (običajno 0)
 * @param out       Kazalec na strukturo, kamor se shranijo rezultati
 * @return ESP_OK, če sta oba parametra uspešno prebrana,
 *         ESP_FAIL, če kateri koli ni uspešen,
 *         ESP_ERR_INVALID_ARG, če je 'out' NULL
 */
esp_err_t soil_read_all(uint16_t cid_offset, soil_data_t *out)
{
    if (!out) return ESP_ERR_INVALID_ARG;

    // Inicijaliziraj strukturo na vse ničle
    memset(out, 0, sizeof(*out));
    
    // Začasne spremenljivke za surove 16-bitne vrednosti
    uint16_t raw_moisture = 0;
    uint16_t raw_temperature = 0;
    
    int success = 0;

    // Preberi vlago
    if (modbus_master_read_uint16(cid_offset + SOIL_CID_MOISTURE, &raw_moisture) == ESP_OK) {
        out->moisture = (float)raw_moisture / 10.0f;
        success++;
    } else {
        ESP_LOGW(TAG, "Branje vlage neuspešno (CID %d)", cid_offset + SOIL_CID_MOISTURE);
    }

    // Preberi temperaturo
    if (modbus_master_read_uint16(cid_offset + SOIL_CID_TEMPERATURE, &raw_temperature) == ESP_OK) {
        // Temperatura je lahko negativna (npr. -5.0°C = -50 v registru)
        // uint16_t ne more predstaviti negativnih vrednosti, zato je potrebna pretvorba
        int16_t signed_temp = (int16_t)raw_temperature;
        out->temperature = (float)signed_temp / 10.0f;
        success++;
    } else {
        ESP_LOGW(TAG, "Branje temperature neuspešno (CID %d)", cid_offset + SOIL_CID_TEMPERATURE);
    }

    // Označi veljavnost – samo če sta oba prebrana
    out->valid = (success == SOIL_CID_COUNT);
    
    ESP_LOGI(TAG, "Prebrano %d/%d parametrov", success, SOIL_CID_COUNT);
    
    return (success == SOIL_CID_COUNT) ? ESP_OK : ESP_FAIL;
}

/**
 * @brief Izpiši vse parametre senzorja v tabeli
 * 
 * @param data Kazalec na strukturo s prebranimi vrednostmi
 */
void soil_print(const soil_data_t *data)
{
    if (!data) return;

    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║          SOIL Senzor (vlaga, temperatura)            ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║  Vlaga:        %5.1f %%                               ║\n", data->moisture);
    printf("║  Temperatura:  %5.1f °C                              ║\n", data->temperature);
    printf("║  Veljavnost:  %s                                     ║\n", data->valid ? "DA" : "NE");
    printf("╚══════════════════════════════════════════════════════╝\n\n");
}