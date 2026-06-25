#pragma once

// ─── WiFi ─────────────────────────────────────────────────────────────────────
#define WIFI_SSID           "ONEfourTWO"
#define WIFI_PASSWORD       "markoskacepozelenitrati"
#define WIFI_MAXIMUM_RETRY  5

// ─── ZLAN Gateway (Modbus TCP strežnik) ───────────────────────────────────────
#define MODBUS_SERVER_IP    "192.168.64.120"
#define MODBUS_TCP_PORT     502

// ─── Slave ID-ji naprav na ZLAN ───────────────────────────────────────────────
// Vsaka naprava priključena na ZLAN RTU port ima svoj Modbus slave naslov
#define SLAVE_ID_DTSU666    11   // CHINT DTSU666-H energijski merilnik
// #define SLAVE_ID_SDM120  12      // Primer: naslednja naprava
// #define SLAVE_ID_SDM630  13

// ─── Modbus Master nastavitve ─────────────────────────────────────────────────
#define MODBUS_TIMEOUT_MS   5000    // Timeout za odgovor slave naprave
#define MODBUS_READ_PERIOD_MS  10000  // Period branja meritev (10 sekund)
