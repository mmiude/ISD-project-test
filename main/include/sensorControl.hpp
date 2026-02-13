#ifndef SENSORCONTROL_HPP
#define SENSORCONTROL_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "Structs.h"
#include "Bme690Task.hpp"
#include "Sps30Task.hpp"
#include "TC74Task.hpp"

/*
// Combined sensor data structure
struct CombinedSensorData {
    BME690Data bme690;
    Sps30Data sps30;
    Tc74a2Data tc74a2;
    
    // Flags to indicate which sensors have provided data
    bool bme690_valid;
    bool sps30_valid;
    bool tc74a2_valid;
    
    // Timestamp when data was collected
    TickType_t timestamp;
};
*/

class sensorControl {
public:
    sensorControl(QueueHandle_t bmv_q, QueueHandle_t bme_q, QueueHandle_t sps_q, QueueHandle_t tc74a2_q, QueueHandle_t output_q);

private:
    QueueHandle_t queue_bmv;
    QueueHandle_t queue_bme;
    QueueHandle_t queue_sps;
    QueueHandle_t queue_tc74a2;
    QueueHandle_t output_queue;  // Queue to send combined data to dummy app

    static void task_wrap(void* pvParameters);
    void task_impl();
};

#endif