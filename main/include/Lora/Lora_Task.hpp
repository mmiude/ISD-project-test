#ifndef LORA_TASK_HPP
#define LORA_TASK_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "sensorControl.hpp"
#include "Structs.h"
#include "LoRaE5.hpp"

#define LORA_TX 17
#define LORA_RX 18

class Lora_Task {
public:
    Lora_Task(QueueHandle_t input_queue);

private:
    QueueHandle_t input_queue;
    static void task_wrap(void* pvParameters);
    void task_impl();
    
    LoRaE5 lora;

    // var for saving AT response
    const char* at_response;
};

#endif