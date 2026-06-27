#pragma once

#include "esp_err.h"
#include "mbcontroller.h"

// ─── CID enum za 4-kanalni 0-20mA modul ──────────────────────────────────────
typedef enum {
    ZQWL_CID_CH1 = 0,    // Kanal 1 (0-20mA)
    ZQWL_CID_CH2,        // Kanal 2
    ZQWL_CID_CH3,        // Kanal 3
    ZQWL_CID_CH4,        // Kanal 4
    ZQWL_CID_COUNT       // Število parametrov (mora biti zadnji)
} zqwl_cid_t;

// ─── Izmerjene vrednosti ──────────────────────────────────────────────────────
typedef struct {
    float channel1;      // Tok v mA (0.00 – 20.00)
    float channel2;
    float channel3;
    float channel4;
    bool  valid;         // true, če so vsi kanali uspešno prebrani
} zqwl_data_t;

/**
 * @brief Vrne kazalec na CID deskriptorsko tabelo za ZQWL modul.
 *
 * @param out_count  Izhodni parameter - število deskriptorjev
 * @return Kazalec na statično tabelo
 */
const mb_parameter_descriptor_t *zqwl_get_descriptors(uint16_t *out_count);

/**
 * @brief Prebere vse 4 kanale iz ZQWL modula.
 *
 * @param cid_offset  Odmik CID-ov (kje v skupni tabeli začne ta naprava)
 * @param out         Struktura, v katero se shranijo vrednosti (v mA)
 * @return ESP_OK ali napaka
 */
esp_err_t zqwl_read_all(uint16_t cid_offset, zqwl_data_t *out);

/**
 * @brief Izpiše vrednosti vseh kanalov na konzolo (ESP_LOGI).
 */
void zqwl_print(const zqwl_data_t *data);