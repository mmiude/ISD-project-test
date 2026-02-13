#ifndef BME690TASK_HPP
#define BME690TASK_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "Bme690.hpp"
#include "driver/i2c_master.h"


class Bme690Task{
public:
    Bme690Task(QueueHandle_t q, i2c_master_dev_handle_t dev_handle);
    static void task_wrap(void *pvParameters);

private:
    void task_impl();
    QueueHandle_t queue;
    i2c_master_dev_handle_t dev_handle_;
};

#endif