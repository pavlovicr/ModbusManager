#pragma once

#include "esp_err.h"

/**
 * @brief Inicializira WiFi v STA načinu in čaka na povezavo.
 *        Blokirajoč klic - vrne se ko je WiFi vzpostavljen ali ko poteče timeout.
 * @return ESP_OK če uspešno, ESP_FAIL sicer
 */
esp_err_t wifi_manager_init(void);

/**
 * @brief Vrne true če je WiFi trenutno povezan
 */
bool wifi_manager_is_connected(void);
