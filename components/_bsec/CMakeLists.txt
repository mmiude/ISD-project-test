idf_component_register(
    SRCS
        "bsec_iaq.c"
        "bsec_integration.c"
        "bme69x.c"
    INCLUDE_DIRS
        "include"

    PRIV_REQUIRES
        driver
        esp_timer
        main
        
)

target_link_libraries(${COMPONENT_LIB} "${CMAKE_CURRENT_LIST_DIR}/libalgobsec.a")