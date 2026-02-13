#include "I2CManager.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "I2CManager";

I2CManager::I2CManager(i2c_cfg cfg) {
    i2c_master_bus_config_t bus_config = {
        .i2c_port =   cfg.port,//I2C_MASTER_NUM,
        .sda_io_num = (gpio_num_t)cfg.sda,
        .scl_io_num = (gpio_num_t)cfg.scl,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags = {
            .enable_internal_pullup = true,
            .allow_pd = false,
        },
    };

    esp_err_t err = i2c_new_master_bus(&bus_config, &bus_handle_);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2C bus: %d", err);
    } else {
        ESP_LOGI(TAG, "I2C bus created on port %d, SDA %d, SCL %d", cfg.port, cfg.sda, cfg.scl);
    }
}

I2CManager::~I2CManager() {
    if (bus_handle_) {
        i2c_del_master_bus(bus_handle_);
        bus_handle_ = nullptr;
    }
}

i2c_master_dev_handle_t I2CManager::addDevice(uint8_t addr, uint32_t freq) {
    if (!bus_handle_) {
        ESP_LOGE(TAG, "Bus not initialized");
        return nullptr;
    }

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = addr,
        .scl_speed_hz = freq,
        .scl_wait_us = 0,
        .flags = {
            .disable_ack_check = false,
        },
    };

    i2c_master_dev_handle_t dev_handle;
    esp_err_t err = i2c_master_bus_add_device(bus_handle_, &dev_config, &dev_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add device 0x%02x: %d", addr, err);
        return nullptr;
    }

    ESP_LOGI(TAG, "Added I2C device 0x%02x", addr);
    return dev_handle;
}

void I2CManager::initBus() {
    if (!bus_handle_) {
        ESP_LOGE(TAG, "Bus not initialized");
        return;
    }
    
    ESP_LOGD(TAG, "Initializing I2C bus by probing all addresses");
    
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .scl_speed_hz = 100000,
    };

    for (uint8_t addr = 1; addr < 127; addr++) {
        dev_cfg.device_address = addr;
        i2c_master_dev_handle_t dev_handle = NULL;
        
        esp_err_t ret = i2c_master_bus_add_device(bus_handle_, &dev_cfg, &dev_handle);
        if (ret == ESP_OK) {
            uint8_t data = 0;
            i2c_master_receive(dev_handle, &data, 1, pdMS_TO_TICKS(50));
            i2c_master_bus_rm_device(dev_handle);
        }
    }
    
    ESP_LOGD(TAG, "I2C bus initialization complete");
}