#ifndef BME690_HPP
#define BME690_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "bme69x.h"
#include "bsec_integration.h"
#include "bsec_iaq.h"
#include "Structs.h"
#include "driver/i2c_master.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

class Bme690{
public:
    Bme690(i2c_master_dev_handle_t dev_handle);
    bool init();
    void setOutputQueue(QueueHandle_t q);

    //bridge functions for output_ready
    static void output_ready(outputs_t* output);
    //bridge for bme69x interface initialization
    static void bme69x_interface_init(struct bme69x_dev *dev, uint8_t intf, uint8_t sen_no);
    //bridges for bsec initialization
    static uint32_t state_load(uint8_t *state_buffer, uint32_t n_buffer);
    static uint32_t config_load(uint8_t *config_buffer, uint32_t n_buffer);
    static uint32_t get_timestamp_ms();
    static void state_save(const uint8_t *state_buffer, uint32_t length);

private:
    //output_ready function
    void handle_output_ready(outputs_t* output);

    //bme690 interface initialization
    static BME69X_INTF_RET_TYPE bme69x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
    static BME69X_INTF_RET_TYPE bme69x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
    static void bme69x_delay_us(uint32_t period, void *intf_ptr);

    //bme69x interface initialization function
    void handle_bme69x_interface_init(struct bme69x_dev *dev, uint8_t intf, uint8_t sen_no);

    //functions needed for bsec initialization
    uint32_t handle_state_load(uint8_t *state_buffer, uint32_t n_buffer);
    uint32_t handle_config_load(uint8_t *config_buffer, uint32_t n_buffer);
    uint32_t handle_get_timestamp_ms();
    void handle_state_save(const uint8_t *state_buffer, uint32_t length);



    //bme690
    bme69x_dev device;
    //bsec
    bsec_version_t version;
    return_values_init ret;

    //store latest retrieved data from the BME690 sensor
    BME690Data latestData;

    QueueHandle_t queue;
    i2c_master_dev_handle_t dev_handle_;
};

#endif