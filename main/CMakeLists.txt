set(srcs app_main.c
    "aquaponics.c")


idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS ""
                   )
