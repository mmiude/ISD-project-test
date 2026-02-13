#include "Bmv080.hpp"

static Bmv080* bmv080_instance = nullptr;


Bmv080::Bmv080(): queue(nullptr)
{
    bmv080_instance = this;
}

#if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
Bmv080::Bmv080(i2c_master_dev_handle_t dev): i2c_dev(dev), queue(nullptr)
{
    bmv080_instance = this;
}
#endif

bool Bmv080::init()
{
    // Initialize communication depending on target
#if defined(CONFIG_IDF_TARGET_ESP32S3)
    // SPI for ESP32S3
    if(!init_spi()){
        stsBMV080 = STATUS_ERROR;
        return false;
    }
#else
    // I2C for ESP32C3/C6 (shared bus, device passed via constructor)
    if (i2c_dev == nullptr) {
        printf("POLVERINE BMV080 ERROR: I2C device handle not set\r\n");
        stsBMV080 = STATUS_ERROR;
        return false;
    }
#endif

    //initialize bmv080
    uint16_t major = 0;
	uint16_t minor = 0;
	uint16_t patch = 0;
	char  	 git_hash[12] = {0};
	int32_t commits_ahead = 0;

    bmv080_delay(1000);

    bmv080_current_status = E_BMV080_OK;
    bmv080_current_status = bmv080_get_driver_version(&major, &minor, &patch, git_hash, &commits_ahead);
    if (bmv080_current_status != E_BMV080_OK)
    {
        printf("POLVERINE BMV080 ERROR: Getting BMV080 sensor driver version failed with BMV080 status %d\r\n", bmv080_current_status);
        stsBMV080 = STATUS_ERROR;
        return false;
    }
    printf("BMV080 sensor driver version: %d.%d.%d.%s.%ld\r\n", major, minor, patch, git_hash, commits_ahead);

    // Open driver with appropriate callbacks
#if defined(CONFIG_IDF_TARGET_ESP32S3)
    bmv080_current_status = bmv080_open(&handle, hspi, bmv080_spi_read_16bit,  bmv080_spi_write_16bit,  bmv080_delay);
#else
    bmv080_current_status = bmv080_open(&handle, i2c_dev, bmv080_i2c_read_16bit,  bmv080_i2c_write_16bit,  bmv080_delay);
#endif
    if(bmv080_current_status != E_BMV080_OK){
        printf("POLVERINE BMV080 ERROR: Initializing BMV080 failed with status %d\r\n", (int)bmv080_current_status);
        stsBMV080 = STATUS_ERROR;
        return false;
  }

    bmv080_current_status = bmv080_reset(handle);
    if (bmv080_current_status != E_BMV080_OK)
    {
        printf("POLVERINE BMV080 ERROR: Resetting BMV080 sensor unit failed with BMV080 status %d\r\n", (int)bmv080_current_status);
        stsBMV080 = STATUS_ERROR;
        return false;
    }   

    /*********************************************************************************************************************
    * Running a particle measurement in duty cycling mode
    *********************************************************************************************************************/
    /* Get default parameter "duty_cycling_period" */
    uint16_t duty_cycling_period = 0;
    bmv080_current_status = bmv080_get_parameter(handle, "duty_cycling_period", (void*)&duty_cycling_period);

    printf("BMV080 Default duty_cycling_period: %d s\r\n", duty_cycling_period);

    /* Set custom parameter "duty_cycling_period" */
    duty_cycling_period = 30;
    bmv080_current_status = bmv080_set_parameter(handle, "duty_cycling_period", (void*)&duty_cycling_period);

    printf("BMV080 Customized duty_cycling_period: %d s\r\n", duty_cycling_period);


    bmv080_current_status = bmv080_start_duty_cycling_measurement(handle,get_tick_ms,E_BMV080_DUTY_CYCLING_MODE_0);
    if(bmv080_current_status != E_BMV080_OK)
    {
        printf("POLVERINE BMV080 ERROR: Starting BMV080 failed with status %d\r\n", (int)bmv080_current_status);
        stsBMV080 = STATUS_ERROR;
        return false;
    }

    stsBMV080 = STATUS_OK;
    printf("POLVERINE BMV080 OK\r\n");


        return true;
}

bool Bmv080::retrieve_data(){
    bmv080_current_status = bmv080_serve_interrupt(handle,bmv080_data_ready,NULL);
    if(bmv080_current_status != E_BMV080_OK)
    {
        printf("Reading BMV080 failed with status %d\r\n", (int)bmv080_current_status);
        return false;
    }
    
    return true;
}

void Bmv080::deinit()
{
    // Only SPI teardown on S3
#if defined(CONFIG_IDF_TARGET_ESP32S3)
    deinit_spi();
#endif
}

void Bmv080::bmv080_data_ready(bmv080_output_t bmv080_output, void* params)
{
    if (bmv080_instance)
        bmv080_instance->handle_data_ready(bmv080_output, params);
}

void Bmv080::handle_data_ready(bmv080_output_t bmv080_output, void* params)
{
    latestData.runtime_sec = bmv080_output.runtime_in_sec;
    latestData.pm1 = bmv080_output.pm1_mass_concentration;
    latestData.pm_2_5_mass = bmv080_output.pm2_5_mass_concentration;
    latestData.pm10_mass = bmv080_output.pm10_mass_concentration;
    latestData.pm2_5_number = bmv080_output.pm2_5_number_concentration;
    latestData.pm10_number = bmv080_output.pm10_number_concentration;
    latestData.is_obstructed = bmv080_output.is_obstructed;
    latestData.is_outside_measurement_range = bmv080_output.is_outside_measurement_range;
    
    /*printf("BMV080:");
    printf("Run time in sec = %.2f second \n", latestData.runtime_sec);
    printf("pm1 = %.2f ug/m3\n", latestData.pm1);
    printf("pm2.5 = %.2f ug/m3\n", latestData.pm_2_5);
    printf("pm10 = %.2f ug/m3\n", latestData.pm10);
    printf("obstructed = %d (%s)\n",
        latestData.is_obstructed,
        latestData.is_obstructed == 0 ? "OK" : "Blocked");
    printf("outside measurement range = %d (%s)\n",
        latestData.is_outside_measurement_range,
        latestData.is_outside_measurement_range == 0 ? "OK" : "Invalid measurement range");*/
    
    if (queue) {
        xQueueOverwrite(queue, &latestData);
    }
}

bool Bmv080::init_spi()
{
    
  esp_err_t comm_status = spi_init(&hspi);
  if(comm_status != ESP_OK)
  {
    printf("Initializing the SPI communication interface failed with status %d\r\n", (int)comm_status);
    return false;
  }
    return true;
}

void Bmv080::deinit_spi()
{
    spi_deinit(&hspi);
}

uint32_t Bmv080::get_tick_ms(void)
{
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

//get queue handle from bmv080task
void Bmv080::setOutputQueue(QueueHandle_t q)
{
    queue = q;
}