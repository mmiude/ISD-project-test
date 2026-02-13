#include "TC74Task.hpp"
#include "TC74Sensor.hpp"
#include "driver/i2c_master.h"
#include "esp_log.h"

extern "C" {
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "freertos/queue.h"
}

TC74Task::TC74Task(QueueHandle_t queue, i2c_master_dev_handle_t dev_handle, uint32_t delay_ms) : dataQueue(queue), i2cdev(dev_handle), delay_ms(delay_ms)
{
    xTaskCreate(taskFunctionWrapper, "TC74Task", 4 * 1024, this, configMAX_PRIORITIES - 1, nullptr);
}


 void TC74Task::taskFunctionWrapper(void *arg)
 {
    static_cast<TC74Task*>(arg)->taskFunction();
 }

void TC74Task::taskFunction()
{
    TC74Sensor TC74(i2cdev);
    Tc74a2Data data;

    while(true) {
        data = TC74.readData();
        xQueueSend(dataQueue, &data, 0);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}