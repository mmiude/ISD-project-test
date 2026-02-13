#include "Bmv080Task.hpp"



Bmv080Task::Bmv080Task(QueueHandle_t q){
    queue = q;
    xTaskCreate(task_wrap,"bmv080_task",60 * 1024, this, configMAX_PRIORITIES - 1, nullptr);

}

#if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
Bmv080Task::Bmv080Task(QueueHandle_t q, i2c_master_dev_handle_t i2c_dev){
    queue = q;
    i2c_dev_ = i2c_dev;
    xTaskCreate(task_wrap,"bmv080_task",60 * 1024, this, configMAX_PRIORITIES - 1, nullptr);
}
#endif

void Bmv080Task::task_wrap(void *pvParameters){
    Bmv080Task* task = static_cast<Bmv080Task*> (pvParameters);
    task->task_impl();
    vTaskDelete(NULL);
}

void Bmv080Task::task_impl(){
    #if defined(CONFIG_IDF_TARGET_ESP32S3)
    Bmv080 bmv;
    #else
    Bmv080 bmv(i2c_dev_);
    #endif
    bmv.setOutputQueue(queue);

    if(!bmv.init()){
        printf("BMV initialization failed");
        vTaskDelete(NULL);
        return;
    }

    while(true){
        bmv080_delay(1000);
        
        if(!bmv.retrieve_data()){
            printf("data retrieving from BMV failed for this round.");
        }
    }

    vTaskDelete(NULL);
}