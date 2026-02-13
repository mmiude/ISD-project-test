#ifndef SPS30TASK_HPP
#define SPS30TASK_HPP

#include "driver/i2c_master.h"
#include "Structs.h"

extern "C" {
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "freertos/queue.h"
}

class Sps30Task 
{
public: 
    Sps30Task(QueueHandle_t queue, i2c_master_dev_handle_t dev_handle, uint32_t delay_ms = 5000);
    static void taskFunctionWrapper(void *arg);

private:
    QueueHandle_t dataQueue;
    i2c_master_dev_handle_t i2cDev;
    uint32_t delay_ms; 

    void taskFunction(); 
};

#endif