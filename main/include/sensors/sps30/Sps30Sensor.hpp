#ifndef SPS30SENSOR_HPP
#define SPS30SENSOR_HPP

#include <memory>
#include <span>
#include <vector>
#include <cstdint>
#include "driver/i2c_master.h"
#include "Structs.h"

class Sps30Sensor {
    public: 
        Sps30Sensor(i2c_master_dev_handle_t dev_handle);
        
        Sps30Data update(); 

    private:
        bool connected{}; 
        const uint8_t address{0x69};
        Sps30Data data;
        i2c_master_dev_handle_t i2cdev;

        bool isSensorConnected();
        bool newDataAvailable();
        static float decode(std::span<const uint8_t, 6> data);
        static bool dataVerified(std::span<const uint8_t> data);
        static uint8_t crc(std::span<const uint8_t> bytes);
};

#endif