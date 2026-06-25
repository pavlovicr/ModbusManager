idf_component_register(
    SRCS "dtsu666.c"
    INCLUDE_DIRS "include"
    REQUIRES modbus_core   # odvisna od skupnega modbus modula
)