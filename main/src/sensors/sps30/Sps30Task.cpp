#include "Sps30Task.hpp"
#include "Sps30Sensor.hpp"
#include "driver/i2c_master.h"
#include "esp_log.h"

extern "C" {
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "freertos/queue.h"
}

//static const char* TAG = "SPS30"; 

Sps30Task::Sps30Task(QueueHandle_t queue, i2c_master_dev_handle_t dev_handle, uint32_t delay_ms) 
    : dataQueue(queue), i2cDev(dev_handle), delay_ms(delay_ms)
{
    xTaskCreate(taskFunctionWrapper, "SPS30Task", 4 * 1024, this, configMAX_PRIORITIES - 1, nullptr);
}

void Sps30Task::taskFunctionWrapper(void *arg) {
    static_cast<Sps30Task*>(arg)->taskFunction(); 
}

void Sps30Task::taskFunction() {
    Sps30Sensor sps30(i2cDev);
    Sps30Data data;

    //printf("starting sps task\n");

    while (true) {
        data = sps30.update(); 
        xQueueSend(dataQueue, &data, 0);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}


