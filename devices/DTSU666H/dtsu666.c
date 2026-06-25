/**
 * @file dtsu666.c
 * @brief DTSU666-H Energijski merilnik - Modbus TCP master gonilnik
 * 
 * Datoteka vsebuje vse funkcije za komunikacijo s CHINT DTSU666-H energijskim
 * merilnikom preko Modbus TCP prehoda (192.168.64.120).
 * 
 * Merilnik uporablja IEEE754 float format s posebnim CDAB zaporedjem bajtov.
 * 
 * @author Rados
 * @date 2026
 */

#include "dtsu666.h"

#include <string.h>
#include "esp_log.h"
#include "modbus_master.h"
#include "app_config.h"

#include "modbus_master.h"

// ─── Oznaka za beleženje ──────────────────────────────────────────────────────
#define TAG "DTSU666"

// ═══════════════════════════════════════════════════════════════════════════════
// REGISTRSKA PRESLIKAVA - DTSU666-H
// ═══════════════════════════════════════════════════════════════════════════════
//
// Format: IEEE754 enojne natančnosti plavajoča vejica (32-bit = 2 Modbus registra)
// Zaporedje bajtov: CDAB (posebna oblika specifična za DTSU666!)
// Modbus funkcija: 03 (Beri držalne registre)
//
// Primer:
//   Register 0x2000-0x2001 = Napetost A-N
//   Vrednost v merilniku: 230,50V (IEEE754)
//   Bajti so v zaporedju CDAB (ne ABCD!)
//   Moram preurediti bajte, da dobim točno vrednost
//
// Tabela registrov:
//
//  Register | Naziv                     | Enota  | Opis
//  ---------|---------------------------|--------|------------------------------------
//  0x2000   | Napetost A-N (L1-Nevtrala)| V      | Linijska napetost faze A
//  0x2002   | Napetost B-N (L2-Nevtrala)| V      | Linijska napetost faze B
//  0x2004   | Napetost C-N (L3-Nevtrala)| V      | Linijska napetost faze C
//  0x2006   | Napetost A-B (L1-L2)      | V      | Medfazna napetost A-B
//  0x2008   | Napetost B-C (L2-L3)      | V      | Medfazna napetost B-C
//  0x200A   | Napetost C-A (L3-L1)      | V      | Medfazna napetost C-A
//  0x200C   | Tok A                     | A      | Efektivni tok faze A
//  0x200E   | Tok B                     | A      | Efektivni tok faze B
//  0x2010   | Tok C                     | A      | Efektivni tok faze C
//  0x2012   | Tok N (nevtralni)         | A      | Tok skozi nevtralni vodnik
//  0x2014   | Aktivna moč A             | W      | Trenutna moč faze A
//  0x2016   | Aktivna moč B             | W      | Trenutna moč faze B
//  0x2018   | Aktivna moč C             | W      | Trenutna moč faze C
//  0x201A   | Aktivna moč (skupna)      | W      | Skupna trofazna moč
//  0x2034   | Frekvenca                 | Hz     | Frekvenca elektroenergetske mreže
//  0x4000   | Aktivna energija (uvoz)   | kWh    | Energija prevzeta iz mreže
//  0x4004   | Aktivna energija (izvoz)  | kWh    | Energija vraćena v mrežo
//
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Deskriptorska tabela za Modbus TCP magistralni ────────────────────────────
//
// Vsak red v tabeli predstavlja en parameter, ki ga želimo brati.
// Polja:
//   .cid                = Edinstvena označba parametra
//   .param_key          = Besedilno ime parametra (za beleženje in razhroščevanje)
//   .param_units        = Merska enota (V, A, W, kWh, Hz...)
//   .mb_slave_addr      = Modbus RTU naslov slave naprave (11 = DTSU666 na ZLAN prehodu)
//   .mb_param_type      = Vrsta registra (MB_PARAM_HOLDING = držalni registri)
//   .mb_reg_start       = Začetni naslov registra na merilniku
//   .mb_size            = Število registrov (2 = 32-bitni float)
//   .param_offset       = Odmik v strukturi (0 = od začetka)
//   .param_type         = Vrsta podatka (PARAM_TYPE_FLOAT)
//   .param_size         = Velikost podatka (PARAM_SIZE_FLOAT = 4 bajti)
//   .param_opts         = Možnosti (najmanj, največ, korak) za preverjanje
//   .access             = Dostop (PAR_PERMS_READ_TRIGGER = beri na zahtevo)
//

static const mb_parameter_descriptor_t s_dtsu666_descriptors[] = {
    // CID                   Ime              Enota  Slave             RegType           Start   Size  Offs  Type             Size             param_opts                          Perms
    // Napetosti (L-N)
    { DTSU_CID_VOLTAGE_A,    "Napetost_A",    "V",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x2000, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=400,    .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_VOLTAGE_B,    "Napetost_B",    "V",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x2002, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=400,    .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_VOLTAGE_C,    "Napetost_C",    "V",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x2004, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=400,    .opt3=1}, PAR_PERMS_READ_TRIGGER },
    
    // Napetosti (L-L)
    { DTSU_CID_VOLTAGE_AB,   "Napetost_AB",   "V",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x2006, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=700,    .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_VOLTAGE_BC,   "Napetost_BC",   "V",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x2008, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=700,    .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_VOLTAGE_CA,   "Napetost_CA",   "V",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x200A, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=700,    .opt3=1}, PAR_PERMS_READ_TRIGGER },
    
    // Toki
    { DTSU_CID_CURRENT_A,    "Tok_A",         "A",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x200C, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=100,    .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_CURRENT_B,    "Tok_B",         "A",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x200E, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=100,    .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_CURRENT_C,    "Tok_C",         "A",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x2010, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=100,    .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_CURRENT_N,    "Tok_N",         "A",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x2012, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=100,    .opt3=1}, PAR_PERMS_READ_TRIGGER },
    
    // Moč
    { DTSU_CID_POWER_A,      "Moc_A",         "W",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x2014, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=-30000, .opt2=30000,  .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_POWER_B,      "Moc_B",         "W",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x2016, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=-30000, .opt2=30000,  .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_POWER_C,      "Moc_C",         "W",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x2018, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=-30000, .opt2=30000,  .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_POWER_TOTAL,  "Moc_skupaj",    "W",   SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x201A, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=-90000, .opt2=90000,  .opt3=1}, PAR_PERMS_READ_TRIGGER },
    
    // Ostalo
    { DTSU_CID_FREQUENCY,    "Frekvenca",     "Hz",  SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x2034, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=45,     .opt2=65,     .opt3=1}, PAR_PERMS_READ_TRIGGER },
    
    // Energija
    { DTSU_CID_ENERGY_IMPORT,"Energija_uvoz", "kWh", SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x4000, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=999999, .opt3=1}, PAR_PERMS_READ_TRIGGER },
    { DTSU_CID_ENERGY_EXPORT,"Energija_izvoz","kWh", SLAVE_ID_DTSU666, MB_PARAM_HOLDING, 0x4004, 2,    0,    PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, {.opt1=0,      .opt2=999999, .opt3=1}, PAR_PERMS_READ_TRIGGER },
};

// Število parametrov v tabeli
static const uint16_t s_descriptor_count =
    sizeof(s_dtsu666_descriptors) / sizeof(s_dtsu666_descriptors[0]);

// ═══════════════════════════════════════════════════════════════════════════════
// POMOŽNE FUNKCIJE
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief Pretvori IEEE754 float iz DTSU666 CDAB oblike v standardno obliko
 * 
 * DTSU666 merilnik uporablja posebno zaporedje bajtov: CDAB
 * Primer:
 *   - Standardno zaporedje ABCD: [A][B][C][D]
 *   - DTSU666 zaporedje CDAB: [C][D][A][B]
 * 
 * Moram  preurediti bajte, da dobim točno IEEE754 vrednost.
 * 
 * Primer pretvorbe:
 *   Vhod (iz merilnika):  [0x43] [0x44] [0x45] [0x46]  (CDAB)
 *   Izhod (pretvorjeno):  [0x45] [0x46] [0x43] [0x44]  (ABCD)
 *   Rezultat: Točna float vrednost (npr. 230,5V)
 * 
 * @param value Kazalec na float vrednost, ki jo trebam pretvoriti
 * 
 * @note Funkcija spremeni vrednost lokalno v pomnilniku
 * @note To se mora izvršiti preden uporabim vrednost!
 */
static void swap_float_bytes(float *value)
{
    if (!value) return;  // Zaščita pred NULL kazalcem
    
    // Pretvori float kazalec v polje uint8_t od 4 bajtov
    uint8_t *bytes = (uint8_t *)value;
    uint8_t temp[4];   // Začasni medpomnilnik za nove bajte
    
    // Preureditev: CDAB → ABCD
    // Vhod:  [C][D][A][B]  (indeksi 0,1,2,3)
    // Izhod: [A][B][C][D]  (indeksi 0,1,2,3)
    temp[0] = bytes[2];  // A prihaja s položaja 2
    temp[1] = bytes[3];  // B prihaja s položaja 3
    temp[2] = bytes[0];  // C prihaja s položaja 0
    temp[3] = bytes[1];  // D prihaja s položaja 1
    
    // Kopiraj preurejenije bajte nazaj na originalno lokacijo
    memcpy(bytes, temp, 4);
}

// ═══════════════════════════════════════════════════════════════════════════════
// JAVNE FUNKCIJE
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief Vrni deskriptorsko tabelo z vsemi parametri merilnika
 * 
 * Ta funkcija vrne niz vseh parametrov, katere merilnik podpira.
 * Uporablja se med inicijalizacijo Modbus TCP magistralnega, da se registrira
 * katera polja trebam brati.
 * 
 * @param out_count Kazalec, kjer se shrani število parametrov
 *                  (Lahko je NULL, če število ni potrebno)
 * 
 * @return Kazalec na deskriptorsko tabelo (s_dtsu666_descriptors)
 * 
 * @example
 *   uint16_t count;
 *   const mb_parameter_descriptor_t *desc = dtsu666_get_descriptors(&count);
 *   ESP_LOGI(TAG, "Merilnik ima %d parametrov", count);
 */
const mb_parameter_descriptor_t *dtsu666_get_descriptors(uint16_t *out_count)
{
    if (out_count) {
        *out_count = s_descriptor_count;
    }
    return s_dtsu666_descriptors;
}

/**
 * @brief Beri vse parametre z merilnika in jih shrani v strukturo
 * 
 * Ta funkcija je osnovna rutina za branje vseh 17 parametrov iz DTSU666 merilnika.
 * Vsak parameter se bere posamično preko Modbus protokola, bajti se pretvorijo
 * iz CDAB v ABCD obliko, in rezultat se shrani v dtsu666_data_t strukturo.
 * 
 * Proces:
 *   1. Zagotovi, da je izhodna struktura veljavna
 *   2. Izbriši celotno strukturo (postavi na 0)
 *   3. Za vsak parameter:
 *      a. Beri float vrednost z Modbus TCP magistralno funkcijo
 *      b. Pretvori CDAB → ABCD zaporedje bajtov
 *      c. Shrani v ustrezno polje strukture
 *      d. Prešteј uspešna branja
 *   4. Postavi .valid zastavico, če so vsi parametri uspešno prebrani
 * 
 * @param cid_offset Začetni CID odmik (običajno 0)
 *                   Uporabi se, če parametri ne začnejo od CID 0
 * @param out       Kazalec na strukturo, kamor se shranijo rezultati
 * 
 * @return ESP_OK, če je vsaj en parameter uspešno prebran
 *         ESP_FAIL, če so vsi parametri neuspešni
 * @return ESP_ERR_INVALID_ARG, če je 'out' NULL
 * 
 * @note Funkcija nadaljuje z branjem tudi, če posamezen parameter ni uspešen
 * @note Beleženje bo pokazalo, kateri so parametri neuspešni
 * 
 * @example
 *   dtsu666_data_t data;
 *   esp_err_t ret = dtsu666_read_all(0, &data);
 *   if (ret == ESP_OK) {
 *       ESP_LOGI(TAG, "Napetost: %.2f V", data.voltage_a);
 *   }
 */
esp_err_t dtsu666_read_all(uint16_t cid_offset, dtsu666_data_t *out)
{
    // Preverjanje vhodnih parametrov
    if (!out) return ESP_ERR_INVALID_ARG;

    // Inicijaliziraj strukturo na vse ničle
    memset(out, 0, sizeof(*out));
    
    // Števec uspešnih branj
    int success = 0;

    // Makro za branje parametra s pretvorbo
    // Makro zmanjša ponavljajočo se kodo - za vsak parameter:
    //   - Beri vrednost preko Modbus-a
    //   - Če je uspešno, pretvori CDAB → ABCD
    //   - Shrani v strukturo
    //   - Prešteј uspeh
    // Če ni uspešno, izpiši opozorilo s podrobnostmi
    #define READ_PARAM(cid_rel, field) \
        if (modbus_master_read_float(cid_offset + (cid_rel), &out->field) == ESP_OK) { \
            swap_float_bytes(&out->field);  /* Pretvori iz CDAB v ABCD */ \
            success++;                       /* Prešteј uspešno branje */ \
        } else { \
            ESP_LOGW(TAG, "Branje CID %u (%s) neuspešno", cid_offset + (cid_rel), #field); \
        }

    // ─── Beri vse parametre ───────────────────────────────────────────────────
    
    // Napetosti (L-N)
    READ_PARAM(DTSU_CID_VOLTAGE_A,    voltage_a)
    READ_PARAM(DTSU_CID_VOLTAGE_B,    voltage_b)
    READ_PARAM(DTSU_CID_VOLTAGE_C,    voltage_c)
    
    // Napetosti (L-L)
    READ_PARAM(DTSU_CID_VOLTAGE_AB,   voltage_ab)
    READ_PARAM(DTSU_CID_VOLTAGE_BC,   voltage_bc)
    READ_PARAM(DTSU_CID_VOLTAGE_CA,   voltage_ca)
    
    // Toki
    READ_PARAM(DTSU_CID_CURRENT_A,    current_a)
    READ_PARAM(DTSU_CID_CURRENT_B,    current_b)
    READ_PARAM(DTSU_CID_CURRENT_C,    current_c)
    READ_PARAM(DTSU_CID_CURRENT_N,    current_n)
    
    // Moč
    READ_PARAM(DTSU_CID_POWER_A,      power_a)
    READ_PARAM(DTSU_CID_POWER_B,      power_b)
    READ_PARAM(DTSU_CID_POWER_C,      power_c)
    READ_PARAM(DTSU_CID_POWER_TOTAL,  power_total)
    
    // Frekvenca
    READ_PARAM(DTSU_CID_FREQUENCY,    frequency)
    
    // Energija
    READ_PARAM(DTSU_CID_ENERGY_IMPORT, energy_import)
    READ_PARAM(DTSU_CID_ENERGY_EXPORT, energy_export)

    // Odstrani makro po uporabi
    #undef READ_PARAM

    // Označi, da so podatki veljavni le, če so vsi parametri uspešno prebrani
    out->valid = (success == DTSU_CID_COUNT);
    
    // Izpiši rezultat branja
    ESP_LOGI(TAG, "Prebrano %d/%d parametrov", success, DTSU_CID_COUNT);
    
    // Vrni uspeh, če je vsaj en parameter prebran
    return (success > 0) ? ESP_OK : ESP_FAIL;
}

/**
 * @brief Izpiši vse parametre merilnika v tabeli
 * 
 * Funkcija formatira in izpiše vse prebrane vrednosti v lepo ASCII tabelo.
 * Uporablja se za hiter pregled stanja merilnika in vseh ključnih parametrov.
 * 
 * Format:
 *   ╔═══════════════════════════════════════════════════════════════════╗
 *   ║     DTSU666-H  Energijski Merilnik                               ║
 *   ║  Napetosti, Toki, Moč, Energija...                               ║
 *   ╚═══════════════════════════════════════════════════════════════════╝
 * 
 * @param data Kazalec na strukturo s prebiranimi vrednostmi
 * 
 * @return Brez povratne vrednosti (void)
 * 
 * @note Če je data NULL, se funkcija takoj vrne brez izpisa
 * @note Vsaka vrstica je formatirana s točnim številom decimalnih mest
 * 
 * @example
 *   dtsu666_data_t metering_data;
 *   dtsu666_read_all(0, &metering_data);
 *   dtsu666_print(&metering_data);  // Izpiši lepo tabelo
 */
void dtsu666_print(const dtsu666_data_t *data)
{
    // Preverjanje veljavnosti kazalca
    if (!data) return;

    // Izpiši tabelo s ASCII okvirom
    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║         DTSU666-H  Energijski Merilnik               ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    
    // Napetosti (L-N)
    printf("║  Napetosti (L-N)                                     ║\n");
    printf("║    A: %8.2f V    B: %8.2f V    C: %8.2f V   ║\n",
           data->voltage_a, data->voltage_b, data->voltage_c);
    
    // Napetosti (L-L)
    printf("║  Napetosti (L-L)                                     ║\n");
    printf("║    AB: %7.2f V   BC: %7.2f V   CA: %7.2f V   ║\n",
           data->voltage_ab, data->voltage_bc, data->voltage_ca);
    
    // Toki
    printf("║  Toki                                                ║\n");
    printf("║    A: %8.3f A    B: %8.3f A    C: %8.3f A   ║\n",
           data->current_a, data->current_b, data->current_c);
    printf("║    N: %8.3f A                                    ║\n",
           data->current_n);
    
    // Moč
    printf("║  Moč                                                 ║\n");
    printf("║    A: %8.1f W    B: %8.1f W    C: %8.1f W   ║\n",
           data->power_a, data->power_b, data->power_c);
    printf("║    Skupaj: %10.1f W                             ║\n",
           data->power_total);
    
    // Frekvenca
    printf("║  Frekvenca: %7.3f Hz                              ║\n",
           data->frequency);
    
    // Energija
    printf("║  Energija  uvoz: %10.3f kWh                     ║\n",
           data->energy_import);
    printf("║  Energija izvoz: %10.3f kWh                     ║\n",
           data->energy_export);
    
    printf("╚══════════════════════════════════════════════════════╝\n\n");
}