#include "TC74Sensor.hpp"

extern "C" {
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "esp_log.h"
}




static const char* TAG = "TC74";

TC74Sensor::TC74Sensor(i2c_master_dev_handle_t dev_handle) : i2cdev(dev_handle)
{
}

Tc74a2Data TC74Sensor::readData(){
    uint8_t rawData = 0;
    uint8_t TEMP = 0x00;
    //uint8_t TEMP = 0x00 // TEMP register
    if (newDataAvailable()) {

        esp_err_t err = i2c_master_transmit(
            i2cdev,
            &TEMP,
            1,
            pdMS_TO_TICKS(100)
        );

        if (err != ESP_OK) {
            ESP_LOGE(TAG, "i2c write failed...\n");
        }
        vTaskDelay(5);

        err = i2c_master_receive(
            i2cdev, 
            &rawData, 
            1,
            pdMS_TO_TICKS(100)
        );

        if (err == ESP_OK) {
        // rawData is a signed 8-bit value
            data.temperature = static_cast<float>(static_cast<int8_t>(rawData)); 
            ESP_LOGI(TAG, "Temperature: %.1f°C\n", data.temperature);
        }
        else {
            ESP_LOGE(TAG, "Failed to read temp %d\n", err);
            data.temperature = -999.0f; // error value?
        }
    }

    return data; 
}


bool TC74Sensor::isSensorConnected()
{ // maybe not needed even. would be called inside the constructor
    return false;
}

bool TC74Sensor::newDataAvailable(){
    uint8_t rawData = 0;
    uint8_t CONFIG = 0x01;

    esp_err_t err = i2c_master_transmit(
        i2cdev,
        &CONFIG,
        1,
        pdMS_TO_TICKS(100)
    );

    if (err != ESP_OK){
        ESP_LOGE(TAG, "Failed to write CONFIG reg\n");
    }

    err = i2c_master_receive(
        i2cdev,
        &rawData,
        1,
        pdMS_TO_TICKS(100)
    );

    if (err != ESP_OK){
        ESP_LOGE(TAG, "Failed to read CONFIG reg\n");
    }

    return ((rawData & (1 << 6)) != 0);
}

void TC74Sensor::setStandByMode(){ // if we want to set the sensor to standby mode these are already implemented. While exiting stanby mode new read bit is automatically 0 so we need to wait a bit
    uint8_t command[] = {0x01, 0x80}; // set bit 7 to one 1000 0000 
    //uint8_t CONFIG
    esp_err_t err = i2c_master_transmit(
        i2cdev,
        command,
        2,
        pdMS_TO_TICKS(100)
    );

    if (err != ESP_OK) ESP_LOGE(TAG, "Failed to set standby mode\n");
    
}

void TC74Sensor::exitStandByMode(){
    uint8_t command[] = {0x01, 0x00};

    esp_err_t err = i2c_master_transmit(
        i2cdev,
        command,
        2,
        pdMS_TO_TICKS(100)
    );

    if (err != ESP_OK) ESP_LOGE(TAG, "Failed to exit standby mode\n");
}