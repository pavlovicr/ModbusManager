
STRUKTURA PROJEKTA

modbus_zlan_master_v1.0/
├── main/
│   ├── main.c
│   ├── app_config.h
│   ├── wifi_manager.c
│   ├── wifi_manager.h
│   └── CMakeLists.txt
├── devices/
│   └── dtsu666/              (ime brez vezaja!)
│       ├── include/
│       │   └── dtsu666.h
│       ├── dtsu666.c
│       ├── README.md
│       └── CMakeLists.txt
├── components/                <--  MAPA za skupne modbus komponente
│   └── modbus_core/
│       ├── include/
│       │   └── modbus_master.h
│       ├── modbus_master.c
│       └── CMakeLists.txt
├── CMakeLists.txt
└── sdkconfig