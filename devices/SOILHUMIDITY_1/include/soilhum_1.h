#pragma once

#include "esp_err.h"
#include "mbcontroller.h"

// ─── CID enum za NPK senzor ────────────────────────────────────────────────────
// CID je globalni indeks v skupni deskriptorski tabeli.
// Vsaka naprava ima svoj blok CID-ov.
typedef enum {
    NPK_CID_NITROGEN = 0,    // Dušik (N)
    NPK_CID_PHOSPHORUS,      // Fosfor (P)
    NPK_CID_POTASSIUM,       // Kalij (K)
    NPK_CID_COUNT            // Mora biti zadnji - pove koliko CID-ov ima ta naprava
} npk_cid_t;

// ─── Izmerjene vrednosti ──────────────────────────────────────────────────────
typedef struct {
    uint16_t nitrogen;    // mg/kg
    uint16_t phosphorus;  // mg/kg
    uint16_t potassium;   // mg/kg
    bool     valid;       // true če so podatki sveži
} npk_data_t;

/**
 * @brief Vrne kazalec na CID deskriptorsko tabelo za NPK senzor.
 *        Uporabi pri gradnji skupne deskriptorske tabele v main.c
 *
 * @param out_count  Izhodni parameter - število deskriptorjev
 * @return Kazalec na statično tabelo
 */
const mb_parameter_descriptor_t *npk_get_descriptors(uint16_t *out_count);

/**
 * @brief Prebere vse registre NPK senzorja iz Modbus Master-ja.
 *
 * @param cid_offset  Odmik CID-ov (= kje v skupni tabeli začne ta naprava)
 * @param out         Struktura v katero se shranijo vrednosti
 * @return ESP_OK ali napaka
 */
esp_err_t npk_read_all(uint16_t cid_offset, npk_data_t *out);

/**
 * @brief Izpiše vrednosti na konzolo (ESP_LOGI)
 */
void npk_print(const npk_data_t *data);