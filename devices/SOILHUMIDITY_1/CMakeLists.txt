idf_component_register(
    SRCS "soilhum_1.c"
    INCLUDE_DIRS "include"
    REQUIRES modbus_core   # odvisna od skupnega modbus modula
)