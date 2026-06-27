#pragma once

// ═══════════════════════════════════════════════════════════════════════════════
//  W I F I   K O N F I G U R A C I J A
// ═══════════════════════════════════════════════════════════════════════════════

#define WIFI_SSID               "ONEfourTWO"
#define WIFI_PASSWORD           "markoskacepozelenitrati"   // ⚠️ V produkciji uporabi Kconfig ali env vars!
#define WIFI_MAXIMUM_RETRY      5

// ═══════════════════════════════════════════════════════════════════════════════
//  M O D B U S   T C P   P R E H O D   ( Z L A N   G A T E W A Y )
// ═══════════════════════════════════════════════════════════════════════════════

#define MODBUS_SERVER_IP        "192.168.64.120"
#define MODBUS_TCP_PORT         502

// ═══════════════════════════════════════════════════════════════════════════════
//  S L A V E   I D - j i   N A P R A V
// ═══════════════════════════════════════════════════════════════════════════════
//
// Vsaka naprava, priključena na ZLAN RTU port, ima svoj Modbus slave naslov.
// Naslove nastavi na napravah samih (DIP stikala ali konfiguracijska zaporedja).

// ─── Energijski merilnik ──────────────────────────────────────────────────────
#define SLAVE_ID_DTSU666        11   // CHINT DTSU666-H (trofazni merilnik)

// ─── Senzor za zemljo (SOIL) ──────────────────────────────────────────────────
#define SLAVE_ID_SOIL            3    // SOIL senzor (vlaga, temperatura zemlje) 

// ─── Serijski RS485 vmesnik za analogne naprave 0-20mA ──────────────────────────────────────
#define SLAVE_ID_ZQWL      20   //  ZQWL_4CH_0_20mA_1 (4-kanalni 0-20mA analogni vhodni modul) 


// ─── Rezervirano za prihodnje naprave ──────────────────────────────────────
// #define SLAVE_ID_SDM120      12   // Eastron SDM120 (enofazni)
// #define SLAVE_ID_SDM630      13   // Eastron SDM630 (trofazni)
// #define SLAVE_ID_ANOTHER     14   // Tvoja naslednja naprava

// ═══════════════════════════════════════════════════════════════════════════════
//  M O D B U S   Č A S O V N I   P A R A M E T R I
// ═══════════════════════════════════════════════════════════════════════════════

#define MODBUS_TIMEOUT_MS       5000    // Timeout za odgovor slave naprave (ms)
#define MODBUS_READ_PERIOD_MS   10000   // Perioda branja meritev (ms)