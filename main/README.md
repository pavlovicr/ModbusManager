
STRUKTURA PROJEKTA

modbus_zlan_master_v1.0/
├── main/
│   ├── main.c
│   ├── app_config.h
│   ├── wifi_manager.c
│   ├── wifi_manager.h
│   ├── modbus_master.c
│   ├── modbus_master.h
│   └── CMakeLists.txt
├── devices/
│   ├── dtsu666/
│   │   ├── include/
│   │   │   └── dtsu666.h
│   │   ├── dtsu666.c
│   │   ├── README.md
│   │   └── CMakeLists.txt
│   ├── sdm120/               # (prihodnja naprava)
│   │   ├── include/
│   │   ├── sdm120.c
│   │   ├── README.md
│   │   └── CMakeLists.txt
│   └── (brez lastnega CMakeLists.txt – ni potreben)
├── CMakeLists.txt            # korenski
└── sdkconfig