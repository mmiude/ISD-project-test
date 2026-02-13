#ifndef BMV080TASK_HPP
#define BMV080TASK_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "Bmv080.hpp"
#if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
#include "driver/i2c_master.h"
#endif

class Bmv080Task{
public:
    Bmv080Task(QueueHandle_t q);
#if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
    Bmv080Task(QueueHandle_t q, i2c_master_dev_handle_t i2c_dev);
#endif
    static void task_wrap(void *pvParameters);

private:
    void task_impl();

    QueueHandle_t queue;
#if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
    i2c_master_dev_handle_t i2c_dev_ = nullptr;
#endif
};

#endif