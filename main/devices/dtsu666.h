#pragma once

#include "esp_err.h"
#include "mbcontroller.h"

// ─── CID enum za DTSU666-H ────────────────────────────────────────────────────
// CID je globalni indeks v skupni deskriptorski tabeli.
// Vsaka naprava ima svoj blok CID-ov - tukaj DTSU začne pri 0.
typedef enum {
    DTSU_CID_VOLTAGE_A = 0,
    DTSU_CID_VOLTAGE_B,
    DTSU_CID_VOLTAGE_C,
    DTSU_CID_VOLTAGE_AB,
    DTSU_CID_VOLTAGE_BC,
    DTSU_CID_VOLTAGE_CA,
    DTSU_CID_CURRENT_A,
    DTSU_CID_CURRENT_B,
    DTSU_CID_CURRENT_C,
    DTSU_CID_CURRENT_N,
    DTSU_CID_POWER_A,
    DTSU_CID_POWER_B,
    DTSU_CID_POWER_C,
    DTSU_CID_POWER_TOTAL,
    DTSU_CID_FREQUENCY,
    DTSU_CID_ENERGY_IMPORT,
    DTSU_CID_ENERGY_EXPORT,
    DTSU_CID_COUNT   // Mora biti zadnji - pove koliko CID-ov ima ta naprava
} dtsu666_cid_t;

// ─── Izmerjene vrednosti ──────────────────────────────────────────────────────
typedef struct {
    float voltage_a;       // V
    float voltage_b;       // V
    float voltage_c;       // V
    float voltage_ab;      // V
    float voltage_bc;      // V
    float voltage_ca;      // V
    float current_a;       // A
    float current_b;       // A
    float current_c;       // A
    float current_n;       // A
    float power_a;         // W
    float power_b;         // W
    float power_c;         // W
    float power_total;     // W
    float frequency;       // Hz
    float energy_import;   // kWh
    float energy_export;   // kWh
    bool  valid;           // true če so podatki sveži
} dtsu666_data_t;

/**
 * @brief Vrne kazalec na CID deskriptorsko tabelo za DTSU666-H.
 *        Uporabi pri gradnji skupne deskriptorske tabele v main.c
 *
 * @param out_count  Izhodni parameter - število deskriptorjev
 * @return Kazalec na statično tabelo
 */
const mb_parameter_descriptor_t *dtsu666_get_descriptors(uint16_t *out_count);

/**
 * @brief Prebere vse registre DTSU666-H iz Modbus Master-ja.
 *
 * @param cid_offset  Odmik CID-ov (= kje v skupni tabeli začne ta naprava)
 * @param out         Struktura v katero se shranijo vrednosti
 * @return ESP_OK ali napaka
 */
esp_err_t dtsu666_read_all(uint16_t cid_offset, dtsu666_data_t *out);

/**
 * @brief Izpiše vrednosti na konzolo (ESP_LOGI)
 */
void dtsu666_print(const dtsu666_data_t *data);
