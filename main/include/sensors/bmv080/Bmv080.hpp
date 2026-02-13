#ifndef BMV080_HPP
#define BMV080_HPP

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "peripherals.h"
#include "bmv080.h"
#include "Structs.h"
#include "bmv080_io.h"
#include "driver/spi_master.h"
#if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
#include "driver/i2c_master.h"
#endif
#include "driver/temperature_sensor.h"
//#include "leds.h"
#include "status.h"
#include "polverine_cfg.h"
#include "freertos/queue.h"


class Bmv080{
public:
    Bmv080();
#if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
    explicit Bmv080(i2c_master_dev_handle_t i2c_dev);
#endif
    bool init();
    bool retrieve_data();
    void deinit();
    void setOutputQueue(QueueHandle_t q);

    //callback bridge
    static void bmv080_data_ready(bmv080_output_t bmv080_output, void* params);


private:
    void handle_data_ready(bmv080_output_t bmv080_output, void* params);

    bool init_spi();
    void deinit_spi();
    static uint32_t get_tick_ms(void);

    spi_device_handle_t hspi;
#if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
    i2c_master_dev_handle_t i2c_dev = nullptr;
#endif
    bmv080_handle_t handle = {0};
    bmv080_status_code_t bmv080_current_status;
    BMV080Data latestData;

    QueueHandle_t queue;
};

#endif
