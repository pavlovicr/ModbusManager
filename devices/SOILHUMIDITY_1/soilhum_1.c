/**
 * @file soilhum_1.c
 * @brief NPK (dušik, fosfor, kalij) senzor - Modbus RTU master gonilnik
 * 
 * Datoteka vsebuje funkcije za komunikacijo z NPK senzorjem preko Modbus RTU.
 * Senzor vrača tri 16-bitne integer vrednosti (mg/kg) na naslovih:
 *   0x001E - Dušik (N)
 *   0x001F - Fosfor (P)
 *   0x0020 - Kalij (K)
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
#define TAG "NPK"

// ═══════════════════════════════════════════════════════════════════════════════
// REGISTRSKA PRESLIKAVA - NPK senzor
// ═══════════════════════════════════════════════════════════════════════════════
//
// Vsi registri so 16-bitni integerji (uint16_t), samo za branje.
// Modbus funkcija: 03 (Beri držalne registre)
//
// Tabela registrov:
//
//  Register | Naziv        | Enota  | Opis
//  ---------|--------------|--------|------------------------------------
//  0x001E   | Dušik (N)    | mg/kg  | Vsebnost dušika v zemlji
//  0x001F   | Fosfor (P)   | mg/kg  | Vsebnost fosforja v zemlji
//  0x0020   | Kalij (K)    | mg/kg  | Vsebnost kalija v zemlji
//
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Deskriptorska tabela za Modbus master ────────────────────────────────
//
// Vsak red v tabeli predstavlja en parameter, ki ga želimo brati.
// Polja:
//   .cid                = Edinstvena označba parametra
//   .param_key          = Besedilno ime parametra (za beleženje)
//   .param_units        = Merska enota
//   .mb_slave_addr      = Modbus naslov slave naprave (definiran v app_config.h)
//   .mb_param_type      = Vrsta registra (MB_PARAM_HOLDING = držalni registri)
//   .mb_reg_start       = Začetni naslov registra na senzorju
//   .mb_size            = Število registrov (1 = 16-bitni integer)
//   .param_offset       = Odmik v strukturi (0 = od začetka)
//   .param_type         = Vrsta podatka (PARAM_TYPE_UINT16)
//   .param_size         = Velikost podatka (PARAM_SIZE_UINT16 = 2 bajta)
//   .param_opts         = Možnosti (najmanj, največ, korak) za preverjanje
//   .access             = Dostop (PAR_PERMS_READ_TRIGGER = beri na zahtevo)
//

static const mb_parameter_descriptor_t s_npk_descriptors[] = {
    // CID               Ime          Enota  Slave            RegType           Start   Size  Offs  Type               Size             param_opts          Perms
    { NPK_CID_NITROGEN,  "Dušik",     "mg/kg", SLAVE_ID_NPK,  MB_PARAM_HOLDING, 0x001E, 1,    0,    PARAM_TYPE_U16, PARAM_SIZE_U16, {.opt1=0, .opt2=1999, .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { NPK_CID_PHOSPHORUS,"Fosfor",    "mg/kg", SLAVE_ID_NPK,  MB_PARAM_HOLDING, 0x001F, 1,    0,    PARAM_TYPE_U16, PARAM_SIZE_U16, {.opt1=0, .opt2=1999, .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { NPK_CID_POTASSIUM, "Kalij",     "mg/kg", SLAVE_ID_NPK,  MB_PARAM_HOLDING, 0x0020, 1,    0,    PARAM_TYPE_U16, PARAM_SIZE_U16, {.opt1=0, .opt2=1999, .opt3=1}, PAR_PERMS_READ_TRIGGER },
};

// Število parametrov v tabeli
static const uint16_t s_descriptor_count =
    sizeof(s_npk_descriptors) / sizeof(s_npk_descriptors[0]);

// ═══════════════════════════════════════════════════════════════════════════════
// JAVNE FUNKCIJE
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief Vrni deskriptorsko tabelo z vsemi parametri senzorja
 * 
 * @param out_count Kazalec, kjer se shrani število parametrov
 * @return Kazalec na deskriptorsko tabelo
 */
const mb_parameter_descriptor_t *npk_get_descriptors(uint16_t *out_count)
{
    if (out_count) {
        *out_count = s_descriptor_count;
    }
    return s_npk_descriptors;
}

/**
 * @brief Beri vse parametre z NPK senzorja in jih shrani v strukturo
 * 
 * Prebere tri 16-bitne registre (N, P, K) in jih shrani v npk_data_t.
 * 
 * @param cid_offset Začetni CID odmik (običajno 0)
 * @param out       Kazalec na strukturo, kamor se shranijo rezultati
 * @return ESP_OK, če so vsi trije parametri uspešno prebrani,
 *         ESP_FAIL, če kateri koli ni uspešen,
 *         ESP_ERR_INVALID_ARG, če je 'out' NULL
 */
esp_err_t npk_read_all(uint16_t cid_offset, npk_data_t *out)
{
    if (!out) return ESP_ERR_INVALID_ARG;

    // Inicijaliziraj strukturo na vse ničle
    memset(out, 0, sizeof(*out));
    
    // Števec uspešnih branj
    int success = 0;

    // Makro za branje 16-bitnega integerja
    #define READ_PARAM_U16(cid_rel, field) \
        if (modbus_master_read_uint16(cid_offset + (cid_rel), &out->field) == ESP_OK) { \
            success++; \
        } else { \
            ESP_LOGW(TAG, "Branje CID %u (%s) neuspešno", cid_offset + (cid_rel), #field); \
        }

    // ─── Beri vse tri parametre ─────────────────────────────────────────────
    READ_PARAM_U16(NPK_CID_NITROGEN,   nitrogen)
    READ_PARAM_U16(NPK_CID_PHOSPHORUS, phosphorus)
    READ_PARAM_U16(NPK_CID_POTASSIUM,  potassium)

    #undef READ_PARAM_U16

    // Označi veljavnost – samo če so vsi trije prebrani
    out->valid = (success == NPK_CID_COUNT);
    
    ESP_LOGI(TAG, "Prebrano %d/%d parametrov", success, NPK_CID_COUNT);
    
    return (success == NPK_CID_COUNT) ? ESP_OK : ESP_FAIL;
}

/**
 * @brief Izpiši vse parametre senzorja v tabeli
 * 
 * @param data Kazalec na strukturo s prebranimi vrednostmi
 */
void npk_print(const npk_data_t *data)
{
    if (!data) return;

    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║          NPK Senzor (dušik, fosfor, kalij)          ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║  Dušik (N):   %5u mg/kg                             ║\n", data->nitrogen);
    printf("║  Fosfor (P):  %5u mg/kg                             ║\n", data->phosphorus);
    printf("║  Kalij (K):   %5u mg/kg                             ║\n", data->potassium);
    printf("║  Veljavnost:  %s                                     ║\n", data->valid ? "DA" : "NE");
    printf("╚══════════════════════════════════════════════════════╝\n\n");
}