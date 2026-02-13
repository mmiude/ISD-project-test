#ifndef DUMMYTASK_HPP
#define DUMMYTASK_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "sensorControl.hpp"

class DummyTask {
public:
    DummyTask(QueueHandle_t input_queue);

private:
    QueueHandle_t input_queue;
    static void task_wrap(void* pvParameters);
    void task_impl();
};

#endif