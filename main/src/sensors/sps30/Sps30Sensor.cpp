#include "Sps30Sensor.hpp"
#include "esp_log.h"

extern "C" {
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
}

static const char* TAG = "SPS30"; 

Sps30Sensor::Sps30Sensor(i2c_master_dev_handle_t dev_handle) : i2cdev(dev_handle)
{
    if (connected = isSensorConnected(); connected){
        //fan cleaning 
        std::vector<uint8_t> command{0x56, 0x07};

        esp_err_t err = i2c_master_transmit(
            i2cdev,
            command.data(),
            command.size(),
            pdMS_TO_TICKS(100)
        );

        vTaskDelay(pdMS_TO_TICKS(10000)); // wait for the fan cleaning

        if (err != ESP_OK) ESP_LOGE(TAG, "SPS30 fan cleaning failed");

        //start measuring mode
        command = {0x00, 0x10};
        std::vector<uint8_t> params{0x03, 0x00};
        command.insert(command.end(), params.begin(), params.end());
        command.emplace_back(crc(params));

        err = i2c_master_transmit(
            i2cdev,
            command.data(),
            command.size(),
            pdMS_TO_TICKS(100)
        );

        vTaskDelay(pdMS_TO_TICKS(1500));

        if (err != ESP_OK) ESP_LOGE(TAG, "SPS30 start measurement failed");

        ESP_LOGI(TAG, "SPS30 initialized"); 

    }
    
}

Sps30Data Sps30Sensor::update() 
{
    if (connected && newDataAvailable()) {

        ESP_LOGI(TAG, "SPS30 connected and new data available");

        constexpr uint8_t RESPONSE_LENGTH = 60;   //total length of the response

        std::vector<uint8_t> command{0x03, 0x00};
        std::vector<uint8_t> response(RESPONSE_LENGTH);

        uint8_t retry = 3;
        bool verified = false; 

        do {

            auto err = i2c_master_transmit(
                i2cdev,
                command.data(),
                command.size(),
                pdMS_TO_TICKS(100)
            );

            vTaskDelay(pdMS_TO_TICKS(50));

            err = i2c_master_receive(
                i2cdev,
                response.data(),
                response.size(),
                pdMS_TO_TICKS(100)
            );


            if (err != ESP_OK) ESP_LOGE(TAG, "SPS30 update failed");

            verified = dataVerified(response);
        } while (!verified && --retry > 0);  // if data verified fails we try three times 

        if (verified) {
            const uint8_t DATA_LENGTH = 6;
            using DataRange = std::span<uint8_t, DATA_LENGTH>;

            uint8_t index = 0;        

            data.mc_1p0 = decode(DataRange{response.begin() + index++ * DATA_LENGTH, DATA_LENGTH});
            data.mc_2p5 = decode(DataRange{response.begin() + index++ * DATA_LENGTH, DATA_LENGTH});
            data.mc_4p0 = decode(DataRange{response.begin() + index++ * DATA_LENGTH, DATA_LENGTH});
            data.mc_10p0 = decode(DataRange{response.begin() + index++ * DATA_LENGTH, DATA_LENGTH});
            data.nc_0p5 = decode(DataRange{response.begin() + index++ * DATA_LENGTH, DATA_LENGTH});
            data.nc_1p0 = decode(DataRange{response.begin() + index++ * DATA_LENGTH, DATA_LENGTH});
            data.nc_2p5= decode(DataRange{response.begin() + index++ * DATA_LENGTH, DATA_LENGTH});
            data.nc_4p0 = decode(DataRange{response.begin() + index++ * DATA_LENGTH, DATA_LENGTH});
            data.nc_10p0 = decode(DataRange{response.begin() + index++ * DATA_LENGTH, DATA_LENGTH});
            data.typical_particle_size = decode(DataRange{response.begin() + index++ * DATA_LENGTH, DATA_LENGTH}); 
        }
    }
    return data; 
} 

bool Sps30Sensor::isSensorConnected()
{
    std::vector<uint8_t> command{0xD1, 0x00};
    std::vector<uint8_t> response(3);

    auto rc = i2c_master_transmit(
        i2cdev,
        command.data(),
        command.size(),
        pdMS_TO_TICKS(100)
    );

    if (rc == ESP_OK) printf("WRITE OK!\n");

    vTaskDelay(pdMS_TO_TICKS(50));

    rc = i2c_master_receive(
        i2cdev,
        response.data(),
        response.size(),
        pdMS_TO_TICKS(100)
    );
    
    if (rc == ESP_OK) printf("READ OK\n");

    return crc(response) == 0;
}

bool Sps30Sensor::newDataAvailable()
{
    uint8_t ready = 0;

    std::vector<uint8_t> command{0x02, 0x02};
    std::vector<uint8_t> response(3);

    auto rc = i2c_master_transmit(
        i2cdev,
        command.data(),
        command.size(),
        pdMS_TO_TICKS(100)
    );

    vTaskDelay(pdMS_TO_TICKS(50));

    rc = i2c_master_receive(
        i2cdev,
        response.data(),
        response.size(),
        pdMS_TO_TICKS(100)
    );

    if (rc != ESP_OK) ESP_LOGE(TAG, "Failed to read i2c");

    if (crc(response) == 0) { ready = response.at(1); }

    return ready == 0x01; 
}
        

float Sps30Sensor::decode(std::span<const uint8_t, 6> data) 
{
    // takes 6 bytes -> stores 4 bytes and ignores 2 crc bytes 
    // SPS30 sends always two bytes followed with crc byte 
    return std::bit_cast<float>(data[0] << 24 | data[1] << 16 | data[3] << 8 | data[4]);
}

bool Sps30Sensor::dataVerified(std::span<const uint8_t> data)
{
    bool failed = false;
    for (uint8_t i = 0; i < data.size() / 3 && !failed; ++i)
    {
        failed = crc(data.subspan(i * 3, 3)) != 0; // pass three bytes to crc check 2 data bytes + 1 crc byte
    }

    return !failed; 
}

uint8_t Sps30Sensor::crc(std::span<const uint8_t> bytes)
{
    uint8_t crc = 0xFF;
    for (auto byte : bytes)
    {
        crc ^= byte;
        for (uint8_t bit = 8; bit > 0; --bit)
        {
            if (crc & 0x80) { crc = (crc << 1) ^ 0x31u; }
            else
            {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}