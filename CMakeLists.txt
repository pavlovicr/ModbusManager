cmake_minimum_required(VERSION 3.16)

# Izključi zastareli freemodbus (ESP-IDF < 5.x) in uporabi esp-modbus
list(APPEND EXTRA_COMPONENT_DIRS "$ENV{IDF_PATH}/components")
set(EXCLUDE_COMPONENTS freemodbus)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(modbus_zlan_master_v1.0)
