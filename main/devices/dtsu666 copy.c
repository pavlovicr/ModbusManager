#include "dtsu666.h"

#include <string.h>
#include "esp_log.h"
#include "modbus_master.h"
#include "app_config.h"

#define TAG "DTSU666"

// ─── Register mapa DTSU666-H ──────────────────────────────────────────────────
// Format: IEEE754 float, 2 registra (32-bit), Big Endian (AB CD)
// Modbus Function Code 03 (Read Holding Registers)
//
//  Naslov  | Parameter         | Enota
//  --------|-------------------|---------
//  0x2000  | Napetost A-N      | V
//  0x2002  | Napetost B-N      | V
//  0x2004  | Napetost C-N      | V
//  0x2006  | Napetost A-B      | V
//  0x2008  | Napetost B-C      | V
//  0x200A  | Napetost C-A      | V
//  0x200C  | Tok A             | A
//  0x200E  | Tok B             | A
//  0x2010  | Tok C             | A
//  0x2012  | Tok N             | A
//  0x2014  | Moč A             | W
//  0x2016  | Moč B             | W
//  0x2018  | Moč C             | W
//  0x201A  | Skupna moč        | W
//  0x2034  | Frekvenca         | Hz
//  0x4000  | Aktivna en. uvoz  | kWh
//  0x4004  | Aktivna en. izvoz | kWh

// ─── CID deskriptorska tabela ─────────────────────────────────────────────────
// mb_parameter_opt_t je union z {int opt1, opt2, opt3} = {int min, max, step}
// OPTS() makro v tej verziji esp-modbus NI definiran — uporabi direkten initializer:
//   .param_opts = {.opt1 = min, .opt2 = max, .opt3 = step}
//
// mb_parameter_descriptor_t polja (v redu):
//   cid, param_key, param_units, mb_slave_addr, mb_param_type,
//   mb_reg_start, mb_size, param_offset, param_type, param_size,
//   param_opts, access

static const mb_parameter_descriptor_t s_dtsu666_descriptors[] = {
    // CID                   Ime              Enota  Slave             RegType           Start   Size  Offs  Type             Size             param_opts                          Perms
    { DTSU_CID_VOLTAGE_A,    "Napetost_A",    "V",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x2000, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=400,    .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_VOLTAGE_B,    "Napetost_B",    "V",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x2002, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=400,    .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_VOLTAGE_C,    "Napetost_C",    "V",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x2004, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=400,    .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_VOLTAGE_AB,   "Napetost_AB",   "V",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x2006, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=700,    .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_VOLTAGE_BC,   "Napetost_BC",   "V",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x2008, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=700,    .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_VOLTAGE_CA,   "Napetost_CA",   "V",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x200A, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=700,    .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_CURRENT_A,    "Tok_A",         "A",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x200C, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=100,    .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_CURRENT_B,    "Tok_B",         "A",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x200E, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=100,    .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_CURRENT_C,    "Tok_C",         "A",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x2010, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=100,    .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_CURRENT_N,    "Tok_N",         "A",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x2012, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=100,    .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_POWER_A,      "Moc_A",         "W",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x2014, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=-30000, .opt2=30000,  .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_POWER_B,      "Moc_B",         "W",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x2016, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=-30000, .opt2=30000,  .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_POWER_C,      "Moc_C",         "W",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x2018, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=-30000, .opt2=30000,  .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_POWER_TOTAL,  "Moc_skupaj",    "W",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x201A, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=-90000, .opt2=90000,  .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_FREQUENCY,    "Frekvenca",     "Hz",  SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x2034, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=45,     .opt2=65,     .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_ENERGY_IMPORT,"Energija_uvoz", "kWh", SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x4000, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=999999, .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_ENERGY_EXPORT,"Energija_izvoz","kWh", SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x4004, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=999999, .opt3=1}, PAR_PERMS_READ_TRIGGER },
};

static const uint16_t s_descriptor_count =
    sizeof(s_dtsu666_descriptors) / sizeof(s_dtsu666_descriptors[0]);

// ─── Javne funkcije ───────────────────────────────────────────────────────────

const mb_parameter_descriptor_t *dtsu666_get_descriptors(uint16_t *out_count)
{
    if (out_count) {
        *out_count = s_descriptor_count;
    }
    return s_dtsu666_descriptors;
}

esp_err_t dtsu666_read_all(uint16_t cid_offset, dtsu666_data_t *out)
{
    if (!out) return ESP_ERR_INVALID_ARG;

    memset(out, 0, sizeof(*out));
    int success = 0;

    // Makro za branje enega parametra in shranjevanje v strukturo
    #define READ_PARAM(cid_rel, field) \
        if (modbus_master_read_float(cid_offset + (cid_rel), &out->field) == ESP_OK) { \
            success++; \
        } else { \
            ESP_LOGW(TAG, "Branje CID %u (%s) neuspešno", cid_offset + (cid_rel), #field); \
        }
       

    READ_PARAM(DTSU_CID_VOLTAGE_A,    voltage_a)
    READ_PARAM(DTSU_CID_VOLTAGE_B,    voltage_b)
    READ_PARAM(DTSU_CID_VOLTAGE_C,    voltage_c)
    READ_PARAM(DTSU_CID_VOLTAGE_AB,   voltage_ab)
    READ_PARAM(DTSU_CID_VOLTAGE_BC,   voltage_bc)
    READ_PARAM(DTSU_CID_VOLTAGE_CA,   voltage_ca)
    READ_PARAM(DTSU_CID_CURRENT_A,    current_a)
    READ_PARAM(DTSU_CID_CURRENT_B,    current_b)
    READ_PARAM(DTSU_CID_CURRENT_C,    current_c)
    READ_PARAM(DTSU_CID_CURRENT_N,    current_n)
    READ_PARAM(DTSU_CID_POWER_A,      power_a)
    READ_PARAM(DTSU_CID_POWER_B,      power_b)
    READ_PARAM(DTSU_CID_POWER_C,      power_c)
    READ_PARAM(DTSU_CID_POWER_TOTAL,  power_total)
    READ_PARAM(DTSU_CID_FREQUENCY,    frequency)
    READ_PARAM(DTSU_CID_ENERGY_IMPORT, energy_import)
    READ_PARAM(DTSU_CID_ENERGY_EXPORT, energy_export)

    

    #undef READ_PARAM

    out->valid = (success == DTSU_CID_COUNT);
    ESP_LOGI(TAG, "Prebrano %d/%d parametrov", success, DTSU_CID_COUNT);
    return (success > 0) ? ESP_OK : ESP_FAIL;
}

void dtsu666_print(const dtsu666_data_t *data)
{
    if (!data) return;

    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║         DTSU666-H  Energijski Merilnik               ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║  Napetosti (L-N)                                     ║\n");
    printf("║    A: %8.2f V    B: %8.2f V    C: %8.2f V   ║\n",
           data->voltage_a, data->voltage_b, data->voltage_c);
    printf("║  Napetosti (L-L)                                     ║\n");
    printf("║    AB: %7.2f V   BC: %7.2f V   CA: %7.2f V   ║\n",
           data->voltage_ab, data->voltage_bc, data->voltage_ca);
    printf("║  Toki                                                ║\n");
    printf("║    A: %8.3f A    B: %8.3f A    C: %8.3f A   ║\n",
           data->current_a, data->current_b, data->current_c);
    printf("║    N: %8.3f A                                    ║\n",
           data->current_n);
    printf("║  Moč                                                 ║\n");
    printf("║    A: %8.1f W    B: %8.1f W    C: %8.1f W   ║\n",
           data->power_a, data->power_b, data->power_c);
    printf("║    Skupaj: %10.1f W                             ║\n",
           data->power_total);
    printf("║  Frekvenca: %7.3f Hz                              ║\n",
           data->frequency);
    printf("║  Energija  uvoz: %10.3f kWh                     ║\n",
           data->energy_import);
    printf("║  Energija izvoz: %10.3f kWh                     ║\n",
           data->energy_export);
    printf("╚══════════════════════════════════════════════════════╝\n\n");
}
