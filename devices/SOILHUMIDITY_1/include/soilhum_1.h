// soilhum_1.h

#pragma once

#include "esp_err.h"
#include "mbcontroller.h"

// ─── CID enum za senzor vlage in temperature ────────────────────────────────
typedef enum {
    SOIL_CID_MOISTURE = 0,    // Relativna vlaga (0-100%), vrednost * 10
    SOIL_CID_TEMPERATURE,     // Temperatura v °C, vrednost * 10
    SOIL_CID_COUNT
} soil_cid_t;

// ─── Izmerjene vrednosti ──────────────────────────────────────────────────────
typedef struct {
    float moisture;    // Izračunana vrednost vlage v % (npr. 25.4)
    float temperature; // Izračunana temperatura v °C (npr. 22.5)
    bool  valid;
} soil_data_t;

// Javne funkcije
const mb_parameter_descriptor_t *soil_get_descriptors(uint16_t *out_count);
esp_err_t soil_read_all(uint16_t cid_offset, soil_data_t *out);
void soil_print(const soil_data_t *data);