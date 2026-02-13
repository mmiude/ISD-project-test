#IDF_TARGET= "esp32s3"
idf_build_get_property(target IDF_TARGET)

message("IDF_TARGET is --------->> ${IDF_TARGET}")

if(${target} STREQUAL "esp32c3" OR ${target} STREQUAL "esp32c6")
    set(BOSCH_LIB
        _bsec
        _bmv080
    )
elseif(${target} STREQUAL "esp32s3")
    set(BOSCH_LIB
        bsec
        bmv080
    )
endif()

message("BOSCH_LIB selsected -------->${BOSCH_LIB}")

idf_component_register(SRCS "src/app/main.cpp"
                            "src/app/sensorControl.cpp"
                            "src/Lora/Lora_Task.cpp"
                            "src/I2CManager.cpp"
                            "src/sensors/bme690/Bme690.cpp" 
                            "src/sensors/bme690/Bme690Task.cpp"
                           
                            "src/sensors/bmv080/Bmv080.cpp" 
                            "src/sensors/bmv080/Bmv080Task.cpp" 
                            "src/sensors/bmv080/bmv080_io.c"
                            
                            "src/sensors/sps30/Sps30Sensor.cpp"
                            "src/sensors/sps30/Sps30Task.cpp"
                            #"src/sensors/sps30/common/sensirion_common.c"
                            #"src/sensors/sps30/common/sensirion_i2c.c"
                            #"src/sensors/sps30/common/sensirion_i2c_hal.c"
                            #"src/sensors/sps30/common/sps30_i2c.c"
                            "src/app/status.cpp"
                            #"src/sensors/Tc74a2/Tc74a2Task.cpp"
                            "src/sensors/TC74/TC74Sensor.cpp"
                            "src/sensors/TC74/TC74Task.cpp"
                            "src/Lora/LoRaE5.cpp"
                            "src/Dummy/DummyTask.cpp"
                           
                            #"src/unused_from_demo/sensorbuffer.c"
                            #"src/unused_from_demo/leds_main.c"
                            #"src/unused_from_demo/usb_main.c"
                            
                       INCLUDE_DIRS 
                       "include" 
                       "include/polverine"
                       "include/sensors"
                       "include/sensors/bme690"
                       "include/Lora"
                       "include/sensors/bmv080"
                       
                       "include/sensors/sps30"
                       "include/sensors/TC74"
                       "include/unused_from_demo"
                       #"src/sensors/sps30/common" 
                         REQUIRES 
                                ${BOSCH_LIB}
                                esp_timer
                                driver
                                esp_wifi
                                esp_system
                                freertos
                                log
                                esp_event
                                esp_netif
                                nvs_flash
                                esp_pm
                                mqtt
                                #i2cdev
                                #bmv080
                                #bsec
                       )
