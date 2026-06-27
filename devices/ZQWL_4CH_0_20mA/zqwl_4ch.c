/**
 * @file zqwl_4ch.c
 * @brief ZQWL 4-kanalni 0-20mA analogni vhodni modul - Modbus RTU master gonilnik
 * 
 * Modul ima 4 analogne vhode za 0-20 mA (ali 4-20 mA) in vrača 16-bitne
 * integer vrednosti, ki jih je treba deliti s 100, da dobimo mA.
 * 
 * @author Rados
 * @date 2026
 */

#include "zqwl_4ch.h"

#include <string.h>
#include "esp_log.h"
#include "modbus_master.h"
#include "app_config.h"

#define TAG "ZQWL"

// ═══════════════════════════════════════════════════════════════════════════════
// REGISTRSKA PRESLIKAVA - ZQWL 4CH 0-20mA
// ═══════════════════════════════════════════════════════════════════════════════
//
// Vsi registri so 16-bitni integerji (uint16_t), samo za branje.
// Modbus funkcija: 03 (Read Holding Registers) – če ne deluje, poskusi z 04.
//
// Tabela registrov:
//
//  Register | Naziv        | Enota  | Opis
//  ---------|--------------|--------|------------------------------------
//  0x0000   | Kanal 1      | mA     | 0-2000 → 0.00-20.00 mA
//  0x0001   | Kanal 2      | mA     | 
//  0x0002   | Kanal 3      | mA     | 
//  0x0003   | Kanal 4      | mA     | 
//
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Deskriptorska tabela ─────────────────────────────────────────────────────
static const mb_parameter_descriptor_t s_zqwl_descriptors[] = {
    { ZQWL_CID_CH1, "Kanal1", "mA", SLAVE_ID_ZQWL, MB_PARAM_HOLDING, 0x0000, 1, 0, PARAM_TYPE_U16, PARAM_SIZE_U16, {.opt1=0, .opt2=2000, .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { ZQWL_CID_CH2, "Kanal2", "mA", SLAVE_ID_ZQWL, MB_PARAM_HOLDING, 0x0001, 1, 0, PARAM_TYPE_U16, PARAM_SIZE_U16, {.opt1=0, .opt2=2000, .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { ZQWL_CID_CH3, "Kanal3", "mA", SLAVE_ID_ZQWL, MB_PARAM_HOLDING, 0x0002, 1, 0, PARAM_TYPE_U16, PARAM_SIZE_U16, {.opt1=0, .opt2=2000, .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { ZQWL_CID_CH4, "Kanal4", "mA", SLAVE_ID_ZQWL, MB_PARAM_HOLDING, 0x0003, 1, 0, PARAM_TYPE_U16, PARAM_SIZE_U16, {.opt1=0, .opt2=2000, .opt3=1}, PAR_PERMS_READ_TRIGGER },
};

static const uint16_t s_descriptor_count = sizeof(s_zqwl_descriptors) / sizeof(s_zqwl_descriptors[0]);

// ═══════════════════════════════════════════════════════════════════════════════
// JAVNE FUNKCIJE
// ═══════════════════════════════════════════════════════════════════════════════

const mb_parameter_descriptor_t *zqwl_get_descriptors(uint16_t *out_count)
{
    if (out_count) {
        *out_count = s_descriptor_count;
    }
    return s_zqwl_descriptors;
}

esp_err_t zqwl_read_all(uint16_t cid_offset, zqwl_data_t *out)
{
    if (!out) return ESP_ERR_INVALID_ARG;

    memset(out, 0, sizeof(*out));

    uint16_t raw[4] = {0};
    int success = 0;

    // Makro za branje posameznega kanala
    #define READ_CH(cid_rel, index) \
        if (modbus_master_read_uint16(cid_offset + (cid_rel), &raw[index]) == ESP_OK) { \
            success++; \
        } else { \
            ESP_LOGW(TAG, "Branje kanala %d neuspešno (CID %d)", index+1, cid_offset + (cid_rel)); \
        }

    READ_CH(ZQWL_CID_CH1, 0)
    READ_CH(ZQWL_CID_CH2, 1)
    READ_CH(ZQWL_CID_CH3, 2)
    READ_CH(ZQWL_CID_CH4, 3)

    #undef READ_CH

    // Pretvorba surovih vrednosti v mA (deljenje s 100)
    if (success > 0) {
        out->channel1 = (float)raw[0] / 100.0f;
        out->channel2 = (float)raw[1] / 100.0f;
        out->channel3 = (float)raw[2] / 100.0f;
        out->channel4 = (float)raw[3] / 100.0f;
    }

    out->valid = (success == ZQWL_CID_COUNT);
    ESP_LOGI(TAG, "Prebrano %d/%d kanalov", success, ZQWL_CID_COUNT);
    return (success == ZQWL_CID_COUNT) ? ESP_OK : ESP_FAIL;
}

void zqwl_print(const zqwl_data_t *data)
{
    if (!data) return;

    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║       ZQWL 4-kanalni 0-20mA vhodni modul            ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║  Kanal 1:  %7.2f mA                                 ║\n", data->channel1);
    printf("║  Kanal 2:  %7.2f mA                                 ║\n", data->channel2);
    printf("║  Kanal 3:  %7.2f mA                                 ║\n", data->channel3);
    printf("║  Kanal 4:  %7.2f mA                                 ║\n", data->channel4);
    printf("║  Veljavnost: %s                                     ║\n", data->valid ? "DA" : "NE");
    printf("╚══════════════════════════════════════════════════════╝\n\n");
}