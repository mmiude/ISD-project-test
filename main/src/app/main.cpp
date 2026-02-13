#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "Bme690Task.hpp"
#include "Bmv080Task.hpp"
#include "Sps30Task.hpp"
#include "sensorControl.hpp"
#include "I2CManager.hpp"
#include "TC74Task.hpp"
#include "LoRaE5.hpp"
#include "Lora_Task.hpp"
#include "DummyTask.hpp"


// Sensor sampling intervals (in milliseconds)
#define SPS30_SAMPLING_INTERVAL_MS 60000  // 1 min for particulate matter
#define TC74A2_SAMPLING_INTERVAL_MS 60000 // 1 min for temperature

extern "C"
{
//#include "sensirion_i2c_hal.h"
}



extern "C" void app_main(void)
{

    #if defined(CONFIG_IDF_TARGET_ESP32S3)
    ESP_LOGI("app_main", "Running on ESP32S3");
    #elif defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
    ESP_LOGI("app_main", "Running on ESP32C3 or ESP32C6");
    #else
    ESP_LOGW("app_main", "No target guarded");
    #endif

    #if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
        i2c_cfg esp_c3_i2c = {
            .port = I2C_MASTER_NUM_CX,
            .sda = (gpio_num_t)I2C_MASTER_SDA_IO_CX,
            .scl = (gpio_num_t)I2C_MASTER_SCL_IO_CX
        };

    static I2CManager i2cManager_cx(esp_c3_i2c);
    
    
    i2c_master_bus_handle_t bus_handle_cx = i2cManager_cx.getBusHandle();
    i2c_master_dev_handle_t bme_dev = i2cManager_cx.addDevice(0x76);    // BME690
    i2c_master_dev_handle_t sps_dev = i2cManager_cx.addDevice(0x69);    // SPS30
    i2c_master_dev_handle_t tc74a2_dev = i2cManager_cx.addDevice(0x4A); // TC74A2
    i2c_master_dev_handle_t bmv_dev = i2cManager_cx.addDevice(0x57);    // BMV080 (I2C)

    // Set the bus and device handles for sensirion_i2c_hal
    sensirion_i2c_hal_set_bus_handle(bus_handle_cx);
    sensirion_i2c_hal_set_device_handle(sps_dev);

    #elif defined(CONFIG_IDF_TARGET_ESP32S3)
    
      i2c_cfg esp_s3_i2c_0 = {
            .port = I2C_MASTER_NUM_0,
            .sda = (gpio_num_t)I2C_MASTER_SDA_IO_0,
            .scl = (gpio_num_t)I2C_MASTER_SCL_IO_0
        };
        i2c_cfg esp_s3_i2c_1 = {
            .port = I2C_MASTER_NUM_1,
            .sda = (gpio_num_t)I2C_MASTER_SDA_IO_1,
            .scl = (gpio_num_t)I2C_MASTER_SCL_IO_1
        };

    static I2CManager i2cManager_0(esp_s3_i2c_0);
    static I2CManager i2cManager_1(esp_s3_i2c_1);

    
    // Initialize I2C bus
    //i2cManager_0.initBus();
    //i2cManager_1.initBus();


    i2c_master_dev_handle_t bme_dev = i2cManager_0.addDevice(0x76);    // BME690

    //i2c_master_bus_handle_t bus_handle_1 = i2cManager_1.getBusHandle();
    i2c_master_dev_handle_t sps_dev = i2cManager_1.addDevice(0x69);    // SPS30
    vTaskDelay(200);
    i2c_master_dev_handle_t tc74a2_dev = i2cManager_1.addDevice(0x4A); // TC74A2

    // Set the bus and device handles for sensirion_i2c_hal
    //sensirion_i2c_hal_set_bus_handle(bus_handle_1);
    //sensirion_i2c_hal_set_device_handle(sps_dev);


 #endif

    // Give I2C bus time to stabilize before starting sensor tasks
    vTaskDelay(pdMS_TO_TICKS(300));

    QueueHandle_t bme_queue = xQueueCreate(1, sizeof(BME690Data));
    QueueHandle_t bmv_queue = xQueueCreate(1, sizeof(BMV080Data));
    QueueHandle_t sps_queue = xQueueCreate(1, sizeof(Sps30Data));
    QueueHandle_t tc74a2_queue = xQueueCreate(1, sizeof(Tc74a2Data));
    QueueHandle_t combined_data_queue = xQueueCreate(5, sizeof(sensor_data));
    
    //static Lora_Task lora_task(combined_data_queue);
    static DummyTask dummy_task(combined_data_queue); // fake lora 
    static Bmv080Task bmv_task(bmv_queue);
    static Bme690Task bme_task(bme_queue, bme_dev);
    static TC74Task tc74a2_task(tc74a2_queue, tc74a2_dev, TC74A2_SAMPLING_INTERVAL_MS);
    static Sps30Task sps_task(sps_queue, sps_dev, SPS30_SAMPLING_INTERVAL_MS);
    static sensorControl sensor_control_task(bmv_queue, bme_queue, sps_queue, tc74a2_queue, combined_data_queue);

    return;
}