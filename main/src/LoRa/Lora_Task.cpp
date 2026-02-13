#include "Lora_Task.hpp"
#include <esp_log.h>

static const char* TAG = "Lora_task";

//LORA_TX & LORA_RX pins configured in lora_task.hpp
Lora_Task::Lora_Task(QueueHandle_t input_queue) : input_queue(input_queue), lora(LORA_TX,LORA_RX) {    

    if (!lora.lora_init()) {
    ESP_LOGE(TAG, "LoRa init failed!");
    return;
    } 
    ESP_LOGI(TAG, "LoRa initialized");

    xTaskCreate(task_wrap, "lora_task", 4096, this, 4, nullptr);
}

void Lora_Task::task_wrap(void* pvParameters) {
    Lora_Task* app = static_cast<Lora_Task*>(pvParameters);
    app->task_impl();
    vTaskDelete(NULL);
}

void Lora_Task::task_impl() {
    sensor_data received_data;

    ESP_LOGI(TAG, "lora task started - waiting for sensor data");

    while (true) {
        if (xQueueReceive(input_queue, &received_data, portMAX_DELAY) == pdPASS) {
            ESP_LOGI(TAG, "**** RECEIVED SENSOR DATA ***");

            switch (received_data.type) {
                case SPS30:
                    ESP_LOGI(TAG, "SPS30 Data:");
                    ESP_LOGI(TAG, "  PM2.5: %.2f µg/m³", received_data.data.sps_data.pm25_mass);
                    ESP_LOGI(TAG, "  PM10: %.2f µg/m³", received_data.data.sps_data.pm10_mass);
                    ESP_LOGI(TAG, "  PM2.5: %.2f particles/cm3", received_data.data.sps_data.pm25_numerical);
                    ESP_LOGI(TAG, "  PM10: %.2f particles/cm3", received_data.data.sps_data.pm10_numerical);
                    lora.send_sensor_data(received_data);
                    break;
                case BME690:
                    ESP_LOGI(TAG, "BME690 Data:");
                    ESP_LOGI(TAG, "  VOC: %.2f", received_data.data.bme_data.voc);
                    ESP_LOGI(TAG, "  Pressure: %.2f Pa", received_data.data.bme_data.pressure);
                    ESP_LOGI(TAG, "  Humidity: %.2f %%", received_data.data.bme_data.humidity);
                    lora.send_sensor_data(received_data);
                    break;
                case TC74A2:
                    ESP_LOGI(TAG, "Temperature Data:");
                    ESP_LOGI(TAG, "  Temperature: %.2f °C", received_data.data.t_data.temperature);
                    lora.send_sensor_data(received_data);
                    break;
                case BMV080:
                    ESP_LOGI(TAG, "BMV080 Data:");
                    ESP_LOGI(TAG, "PM2.5_mass: %.2f µg/m3", received_data.data.bmv_data.pm25_mass);
                    ESP_LOGI(TAG, "PM10_mass: %.2f µg/m3", received_data.data.bmv_data.pm10_mass);
                    ESP_LOGI(TAG, "PM2.5_number: %.2f particles/cm3", received_data.data.bmv_data.pm25_numerical);
                    ESP_LOGI(TAG, "PM10_number: %.2f particles/cm3", received_data.data.bmv_data.pm10_numerical);
                    lora.send_sensor_data(received_data);
                    break;
                    default:
                    ESP_LOGW(TAG, "Unknown sensor data type: %d", received_data.type);
                    break;
            }
            ESP_LOGI(TAG, "=====================================");
        }
    }
}