#ifndef I2CMANAGER_HPP
#define I2CMANAGER_HPP

#include "driver/i2c_master.h"
#include <stdint.h>

#ifdef CONFIG_IDF_TARGET_ESP32C3
#define I2C_MASTER_NUM_CX I2C_NUM_0
#define I2C_MASTER_SDA_IO_CX 8
#define I2C_MASTER_SCL_IO_CX 9
#elif defined(CONFIG_IDF_TARGET_ESP32C6)
#define I2C_MASTER_NUM_CX I2C_NUM_0
#define I2C_MASTER_SDA_IO_CX 4
#define I2C_MASTER_SCL_IO_CX 5
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
#define I2C_MASTER_NUM_0 I2C_NUM_0
#define I2C_MASTER_NUM_1 I2C_NUM_1
#define I2C_MASTER_SDA_IO_0 14
#define I2C_MASTER_SCL_IO_0 21
#define I2C_MASTER_SDA_IO_1 8
#define I2C_MASTER_SCL_IO_1 9
#else
#error "Unsupported target: only ESP32-C3, ESP32-C6, and ESP32-S3 are supported"
#endif


#define I2C_MASTER_FREQ_HZ 100000

typedef struct {
    i2c_port_t port;
    gpio_num_t sda; 
    gpio_num_t scl; 
} i2c_cfg;

class I2CManager {
public:
    I2CManager(i2c_cfg cfg);
    ~I2CManager();

    i2c_master_dev_handle_t addDevice(uint8_t addr, uint32_t freq = I2C_MASTER_FREQ_HZ);
    i2c_master_bus_handle_t getBusHandle() { return bus_handle_; }
    
    // Initialize I2C bus by probing all addresses to clear stuck states
    void initBus();

private:
    i2c_master_bus_handle_t bus_handle_;
};

#endif