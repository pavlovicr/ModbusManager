idf_component_register(
    SRCS "zqwl_4ch.c"
    INCLUDE_DIRS "include"
    REQUIRES modbus_core   # odvisna od skupnega modbus modula
)