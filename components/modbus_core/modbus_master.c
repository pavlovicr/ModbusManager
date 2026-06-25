#include "modbus_master.h"

#include "esp_log.h"
#include "mbcontroller.h"
#include "app_config.h"

#define TAG "MB_MASTER"

// Pomožna makro pretvorba numeričnega MODBUS_TCP_PORT v niz (potrebno v IP tabeli spodaj)
#define _MB_STR(x) #x
#define MB_STR(x)  _MB_STR(x)

static void *s_master_handle = NULL;

// IP tabela za ZLAN gateway.
// POMEMBNO (sprememba v esp-modbus v2.x): vsak vnos mora biti v obliki
// "UID;ip_ali_hostname;port", kjer je UID slave naslov (mb_slave_addr iz
// deskriptorske tabele) - NI IP naslov. Ker gre ZLAN gateway skozi en IP,
// a streže več slave ID-jev, potrebuješ po eno vrstico za VSAK unikaten
// mb_slave_addr, ki ga uporabljaš v `descriptor`.
// Spodaj je primer za slave ID-je 1 in 2 - PRILAGODI glede na svojo tabelo!
static const char *s_tcp_ip_table[] = {
    "11;" MODBUS_SERVER_IP ";" MB_STR(MODBUS_TCP_PORT),
    "2;" MODBUS_SERVER_IP ";" MB_STR(MODBUS_TCP_PORT),
    NULL
};

esp_err_t modbus_master_init(const mb_parameter_descriptor_t *descriptor, uint16_t num_params)
{
    // 1. Komunikacijska konfiguracija
    //    esp-modbus >= 2.0: mb_communication_info_t je UNION, TCP polja so pod .tcp_opts
    mb_communication_info_t comm_info = {
        .tcp_opts.port = MODBUS_TCP_PORT,
        .tcp_opts.mode = MB_TCP,
        .tcp_opts.addr_type = MB_IPV4,
        .tcp_opts.ip_addr_table = (void *)s_tcp_ip_table,
        .tcp_opts.ip_netif_ptr = NULL,   // NULL = privzeti netif
        //.tcp_opts.timeout_ms = MODBUS_TIMEOUT_MS, // timeout za odgovor slave naprave
    };

    // 2. Konstruktor: v v2.x mbc_master_create_tcp() naredi init IN setup v enem
    //    klicu in vrne handle preko drugega parametra (stara mbc_master_init_tcp +
    //    mbc_master_setup sta ukinjena)
    esp_err_t err = mbc_master_create_tcp(&comm_info, &s_master_handle);
    if (err != ESP_OK || s_master_handle == NULL) {
        ESP_LOGE(TAG, "Inicializacija Modbus TCP Master neuspešna: %s", esp_err_to_name(err));
        return err;
    }

    // 3. Nastavi CID deskriptorsko tabelo (vse naprave skupaj)
    //    POZOR: zdaj zahteva handle kot prvi argument
    err = mbc_master_set_descriptor(s_master_handle, descriptor, num_params);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Set descriptor neuspešen: %s", esp_err_to_name(err));
        return err;
    }

    // 4. Zaženi stack (zdaj zahteva handle)
    err = mbc_master_start(s_master_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Start Modbus Master neuspešen: %s", esp_err_to_name(err));
        return err;
    
    }

    ESP_LOGI(TAG, "Modbus TCP Master inicializiran (gateway: %s:%d, %d parametrov)",
             MODBUS_SERVER_IP, MODBUS_TCP_PORT, num_params);
    return ESP_OK;
}


esp_err_t modbus_master_read_float(uint16_t cid, float *value)
{
    
    if (s_master_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t type = 0;
    // V v2.x ni več ločenega parametra za velikost - velikost (4 B za float)
    // esp-modbus vzame neposredno iz param_size v deskriptorski tabeli za ta CID.
    esp_err_t err = mbc_master_get_parameter(s_master_handle, cid, (uint8_t *)value, &type);

    if (err != ESP_OK) {
        ESP_LOGD(TAG, "CID %u branje neuspešno: %s", cid, esp_err_to_name(err));
    }
    
       
    
    return err;
}

void modbus_master_deinit(void)
{
    if (s_master_handle != NULL) {
        // mbc_master_destroy() v v2.x naredi stop + sprostitev vseh virov v enem klicu
        mbc_master_stop(s_master_handle);
        //mbc_master_destroy(s_master_handle);
        s_master_handle = NULL;
        ESP_LOGI(TAG, "Modbus Master zaustavljen");
    }
}