#ifndef TC74SENSOR_HPP
#define TC74SENSOR_HPP

#include <memory>
#include <span>
#include <vector>
#include <cstdint>
#include "driver/i2c_master.h"
#include "Structs.h"

class TC74Sensor
{
public:
    TC74Sensor(i2c_master_dev_handle_t dev_handle);

    Tc74a2Data readData(); 

private:
    bool connected{}; 
    const uint8_t address{0x4A};
  
    Tc74a2Data data;
    i2c_master_dev_handle_t i2cdev;

    bool isSensorConnected();
    bool newDataAvailable();
    void setStandByMode();
    void exitStandByMode();
};

#endif