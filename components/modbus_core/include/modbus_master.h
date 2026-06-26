#pragma once

#include "esp_err.h"
#include "mbcontroller.h"   // esp-modbus



esp_err_t modbus_master_read_uint16(uint16_t cid, uint16_t *out);//deklaracija funkcije za branje 16-bitnega unsigned integerja iz Modbus Master-ja po CID

/**
 * @brief Inicializira Modbus TCP Master z esp-modbus knjižnico.
 *        Kliči enkrat po wifi_manager_init().
 *
 * @param descriptor    Tabela CID deskriptorjev (vse naprave skupaj)
 * @param num_params    Število vnosov v tabeli
 * @return ESP_OK ali napaka
 */
esp_err_t modbus_master_init(const mb_parameter_descriptor_t *descriptor, uint16_t num_params);

/**
 * @brief Prebere en parameter po CID iz ustreznega slave-a.
 *
 * @param cid       CID iz deskriptorske tabele
 * @param value     Kazalec na float kjer se shrani rezultat
 * @return ESP_OK ali napaka
 */
esp_err_t modbus_master_read_float(uint16_t cid, float *value);

/**
 * @brief Zaustavi in sprosti Modbus Master
 */
void modbus_master_deinit(void);
