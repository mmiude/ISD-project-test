#ifndef TC74TASK_HPP
#define TC74TASK_HPP

#include "driver/i2c_master.h"
#include "Structs.h"

extern "C" {
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "freertos/queue.h"
}

class TC74Task 
{
public: 
    TC74Task(QueueHandle_t queue, i2c_master_dev_handle_t dev_handle, uint32_t delay_ms = 5000);
    static void taskFunctionWrapper(void *arg);

private:
    QueueHandle_t dataQueue;
    i2c_master_dev_handle_t i2cdev;
    uint32_t delay_ms;

    void taskFunction();
};

#endif