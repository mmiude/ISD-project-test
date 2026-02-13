#include "sensorControl.hpp"
#include <esp_log.h>

static const char* TAG = "SensorControl";
sensor_data convert_bmv080_to_sensor_data(const BMV080Data& bmv) {
    sensor_data msg;
    msg.type = BMV080;
    msg.data.bmv_data.pm25_mass = bmv.pm_2_5_mass;
    msg.data.bmv_data.pm10_mass = bmv.pm10_mass;
    msg.data.bmv_data.pm25_numerical = bmv.pm2_5_number;
    msg.data.bmv_data.pm10_numerical = bmv.pm10_number;
    return msg;
}
    

sensor_data convert_sps30_to_sensor_data(const Sps30Data& sps30) {
    sensor_data msg;
    msg.type = SPS30;
    msg.data.sps_data.pm25_mass = sps30.mc_2p5;
    msg.data.sps_data.pm10_mass = sps30.mc_10p0;
    msg.data.sps_data.pm25_numerical = sps30.nc_2p5;
    msg.data.sps_data.pm10_numerical = sps30.nc_10p0;
    return msg;
}

sensor_data convert_bme690_to_sensor_data(const BME690Data& bme690) {
    sensor_data msg;
    msg.type = BME690;
    msg.data.bme_data.voc = bme690.breath_voc;
    msg.data.bme_data.pressure = bme690.raw_pressure;
    msg.data.bme_data.humidity = bme690.humidity;
    return msg;
}

sensor_data convert_tc74a2_to_sensor_data(const Tc74a2Data& tc74a2) {
    sensor_data msg;
    msg.type = TC74A2;
    msg.data.t_data.temperature = tc74a2.temperature;
    return msg;
}

sensorControl::sensorControl(QueueHandle_t bmv_q, QueueHandle_t bme_q, QueueHandle_t sps_q, QueueHandle_t tc74a2_q, QueueHandle_t output_q){
    queue_bmv = bmv_q;
    queue_bme = bme_q;
    queue_sps = sps_q;
    queue_tc74a2 = tc74a2_q;
    output_queue = output_q;

    xTaskCreate(task_wrap, "sensor_control_task", 4096, this, 5, nullptr);
}


void sensorControl::task_wrap(void* pvParameters)
{
    sensorControl* task = static_cast<sensorControl*> (pvParameters);
    task->task_impl();
    vTaskDelete(NULL);
}

void sensorControl::task_impl()
{
    CombinedSensorData combined_data = {0};
    TickType_t last_send_time = xTaskGetTickCount();
    const TickType_t SEND_INTERVAL = pdMS_TO_TICKS(120000); // send interval is 2 min (for testing).

    ESP_LOGI(TAG, "Sensor control task started");

    while (true)
    {
        TickType_t current_time = xTaskGetTickCount();

        // --- BME690 ---
        if (queue_bme != nullptr && xQueueReceive(queue_bme, &combined_data.bme690, 0) == pdPASS)
        {
            combined_data.bme690_valid = true;
            combined_data.timestamp = current_time;
            ESP_LOGI(TAG, "Received BME690 data");
        }

        // --- SPS30 ---
        if (queue_sps != nullptr && xQueueReceive(queue_sps, &combined_data.sps30, 0) == pdPASS)
        {
            combined_data.sps30_valid = true;
            combined_data.timestamp = current_time;
            ESP_LOGI(TAG, "Received SPS30 data");
        }

        // --- TC74A2 ---
        if (queue_tc74a2 != nullptr && xQueueReceive(queue_tc74a2, &combined_data.tc74a2, 0) == pdPASS)
        {
            combined_data.tc74a2_valid = true;
            combined_data.timestamp = current_time;
            ESP_LOGI(TAG, "Received TC74A2 data");
        }

        // --- BMV080 ---
        if (queue_bmv != nullptr && xQueueReceive(queue_bmv, &combined_data.bmv080, 0) == pdPASS)
        {
            combined_data.bmv080_valid = true;
            combined_data.timestamp = current_time;
            ESP_LOGI(TAG, "Received BMV080 data");
        }

        // Check if all sensors have valid data
        bool all_sensors_valid = combined_data.bme690_valid &&
                                combined_data.sps30_valid &&
                                combined_data.tc74a2_valid &&
                                combined_data.bmv080_valid;

        if (all_sensors_valid || (current_time - last_send_time) >= SEND_INTERVAL)
        {   
            // Send individual sensor_data messages for DummyApp
            if (combined_data.sps30_valid) {
                sensor_data sps_msg = convert_sps30_to_sensor_data(combined_data.sps30);
                xQueueSend(output_queue, &sps_msg, pdMS_TO_TICKS(100));
            }

            if (combined_data.bme690_valid) {
                sensor_data bme_msg = convert_bme690_to_sensor_data(combined_data.bme690);
                xQueueSend(output_queue, &bme_msg, pdMS_TO_TICKS(100));
            }
            
            if (combined_data.tc74a2_valid) {
                sensor_data temp_msg = convert_tc74a2_to_sensor_data(combined_data.tc74a2);
                xQueueSend(output_queue, &temp_msg, pdMS_TO_TICKS(100));
            }

            if (combined_data.bmv080_valid) {
                sensor_data bmv_msg = convert_bmv080_to_sensor_data(combined_data.bmv080);
                xQueueSend(output_queue, &bmv_msg, pdMS_TO_TICKS(100));
            }

                ESP_LOGI(TAG, "Sent sensor data - BME690:%s SPS30:%s TC74A2:%s BMV080:%s",
                    combined_data.bme690_valid ? "VALID" : "INVALID",
                    combined_data.sps30_valid ? "VALID" : "INVALID",
                    combined_data.tc74a2_valid ? "VALID" : "INVALID",
                    combined_data.bmv080_valid ? "VALID" : "INVALID");

            combined_data.bme690_valid = false;
            combined_data.sps30_valid = false;
            combined_data.tc74a2_valid = false;
            combined_data.bmv080_valid = false;
            last_send_time = current_time;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
