#include "Bme690Task.hpp"
#include "bsec_integration.h"
#include "freertos/task.h"
#include <stdio.h>
#include "status.h"
//#include "leds.h"

extern "C" {
    extern uint8_t *bsecInstance[NUM_OF_SENS];
}

Bme690Task::Bme690Task(QueueHandle_t q, i2c_master_dev_handle_t dev_handle) : queue(q), dev_handle_(dev_handle) {
    xTaskCreate(task_wrap, "bme690_task",60 * 1024 ,this,configMAX_PRIORITIES - 1,nullptr);
}

void Bme690Task::task_wrap(void *pvParameters){
    Bme690Task* task = static_cast<Bme690Task*>(pvParameters);
    task ->task_impl();
    vTaskDelete(nullptr);
}

void Bme690Task::task_impl(){
    //create sensor object
    Bme690 bme(dev_handle_);

    bme.setOutputQueue(queue);

    if (!bme.init()) {
        printf("BME690 init FAILED - sensor not responding\n");
        stsBME690 = STATUS_ERROR;
        // Don't delete task, just mark as failed and exit
        return;
    }
    
    bsec_version_t version;
    return_values_init ret = {BME69X_OK, BSEC_OK};

    // Feed watchdog during BSEC initialization
    vTaskDelay(pdMS_TO_TICKS(10)); // Small delay to allow other tasks
    
    ret = bsec_iot_init(SAMPLE_RATE,Bme690::bme69x_interface_init,Bme690::state_load,Bme690::config_load);

	if (ret.bme69x_status != BME69X_OK) {
		printf("POLVERINE BME690 ERROR while initializing BME68x: %d\r\n", ret.bme69x_status);
        stsBME690 = STATUS_ERROR;
        return; // Don't delete task
	}
	if (ret.bsec_status < BSEC_OK) {
		printf("POLVERINE BME690 ERROR while initializing BSEC library: %d\r\n", ret.bsec_status);
        stsBME690 = STATUS_ERROR;
        return; // Don't delete task
	}
	else if (ret.bsec_status > BSEC_OK) {
		printf("POLVERINE BME690 WARNING while initializing BSEC library: %d\r\n", ret.bsec_status);
	}

	ret.bsec_status = bsec_get_version(bsecInstance, &version);

    printf("POLVERINE BME690 OK\r\n");
    printf("BSEC Version : %u.%u.%u.%u\r\n",version.major,version.minor,version.major_bugfix,version.minor_bugfix);
    stsBME690 = STATUS_OK;

    // Main BSEC loop - this runs indefinitely
    bsec_iot_loop(Bme690::state_save, Bme690::get_timestamp_ms, Bme690::output_ready);

    //deinit i2c if there is mistake
    // bme.deinit_i2c();
    vTaskDelete(NULL);

}

