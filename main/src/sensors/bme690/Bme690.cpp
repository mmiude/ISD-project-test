#include "Bme690.hpp"

//bme690 instance
static Bme690* bme690_instance = nullptr;


//bridge functions for C interface
void Bme690::output_ready(outputs_t* output){
    if(bme690_instance){
        bme690_instance->handle_output_ready(output);
    }
}

void Bme690::bme69x_interface_init(struct bme69x_dev *dev, uint8_t intf, uint8_t sen_no){
    if(bme690_instance){
        bme690_instance->handle_bme69x_interface_init(dev,intf,sen_no);
    }
}

uint32_t Bme690::state_load(uint8_t *state_buffer, uint32_t n_buffer){
    if(bme690_instance){
        return bme690_instance->handle_state_load(state_buffer,n_buffer);
    }
    return 0;
}

uint32_t Bme690::config_load(uint8_t *config_buffer, uint32_t n_buffer){
    if(bme690_instance){
        return bme690_instance->handle_config_load(config_buffer,n_buffer);
    }
    return 0;
}

uint32_t Bme690::get_timestamp_ms(){
    if(bme690_instance){
        return bme690_instance->handle_get_timestamp_ms();
    }
    return 0;
}

void Bme690::state_save(const uint8_t *state_buffer, uint32_t length){
    if(bme690_instance){
        bme690_instance->handle_state_save(state_buffer,length);
    }
}


Bme690::Bme690(i2c_master_dev_handle_t dev_handle): queue(nullptr), dev_handle_(dev_handle){
    bme690_instance = this;
}

bool Bme690::init(){
    // Test I2C communication by reading chip ID
    uint8_t chip_id;
    BME69X_INTF_RET_TYPE ret = bme69x_i2c_read(BME69X_REG_CHIP_ID, &chip_id, 1, nullptr);
    
    if (ret != BME69X_INTF_RET_SUCCESS || chip_id != BME69X_CHIP_ID) {
        printf("BME690: Failed to read chip ID (got 0x%02x, expected 0x%02x)\n", chip_id, BME69X_CHIP_ID);
        return false;
    }
    
    printf("BME690: Chip ID verified (0x%02x)\n", chip_id);
    return true;
}


BME69X_INTF_RET_TYPE Bme690::bme69x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    (void)intf_ptr;

    esp_err_t err = i2c_master_transmit_receive(bme690_instance->dev_handle_, &reg_addr, 1, reg_data, len, -1/*pdMS_TO_TICKS(100)*/);
    return (err == ESP_OK) ? BME69X_INTF_RET_SUCCESS : BME69X_E_COM_FAIL;
}

BME69X_INTF_RET_TYPE Bme690::bme69x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    static uint8_t buffer[257];

        (void)intf_ptr;

    if(len < 256)
    {
        buffer[0] = reg_addr;
        memcpy(&buffer[1],reg_data,len);
        esp_err_t err = i2c_master_transmit(bme690_instance->dev_handle_, buffer, len+1, pdMS_TO_TICKS(100));
        return (err == ESP_OK) ? BME69X_INTF_RET_SUCCESS : BME69X_E_COM_FAIL;
    }
    else
        return BME69X_E_COM_FAIL;
}

void Bme690::bme69x_delay_us(uint32_t period, void *intf_ptr)
{
    (void)intf_ptr;
    vTaskDelay(period / (portTICK_PERIOD_MS*1000));
}

void Bme690::handle_bme69x_interface_init(struct bme69x_dev *bme, uint8_t intf, uint8_t sen_no)
{
    if (bme == NULL)
        return;

    /* Bus configuration : I2C */
        //dev_addr[sen_no] = BME69X_I2C_ADDR_LOW;
        bme->read = bme69x_i2c_read;
        bme->write = bme69x_i2c_write;
        bme->intf = BME69X_I2C_INTF;
        bme->delay_us = bme69x_delay_us;
        bme->intf_ptr = 0; //&dev_addr[sen_no];
        bme->amb_temp = 22; /* The ambient temperature in deg C is used for defining the heater temperature */

}

uint32_t Bme690::handle_state_load(uint8_t *state_buffer, uint32_t n_buffer)
{
    // ...
    // Load a previous library state from non-volatile memory, if available.
    //
    // Return zero if loading was unsuccessful or no state was available,f
    // otherwise return length of loaded state string.
    // ...
    return 0;
}

uint32_t Bme690::handle_config_load(uint8_t *config_buffer, uint32_t n_buffer)
{
	memcpy(config_buffer, bsec_config_iaq, n_buffer);
    return n_buffer;
}

uint32_t Bme690::handle_get_timestamp_ms()
{
     uint32_t system_current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

    return system_current_time;
}

void Bme690::handle_state_save(const uint8_t *state_buffer, uint32_t length)
{
    // ...
    // Save the string some form of non-volatile memory, if possible.
    // ...
}

void Bme690::handle_output_ready(outputs_t* output){
    latestData.temperature = output->compensated_temperature;
    latestData.raw_pressure = output->raw_pressure;
    latestData.humidity = output->compensated_humidity;
    latestData.iaq = output->iaq;
    latestData.iaq_accuracy = output->iaq_accuracy;
    latestData.co2 = output->co2_equivalent;
    latestData.breath_voc = output->breath_voc_equivalent;
    latestData.stabStatus = output->stabStatus;
    latestData.runInStatus = output->runInStatus;

    /*printf("BME690:");
    printf("T = %.2f C\n", latestData.temperature);
    printf("P = %.2f Pa\n", latestData.raw_pressure);
    printf("H = %.2f %%\n", latestData.humidity);
    printf("IAQ = %.2f\n", latestData.iaq);
    printf("IAQ accuracy = %d\n", (int)latestData.iaq_accuracy);
    printf("CO2 = %.2f ppm\n", latestData.co2);
    printf("VOC = %.2f\n", latestData.breath_voc);
    printf("raw_gas = %.2f\n", output->raw_gas);
    printf("gas_percentage = %.2f\n", output->gas_percentage);
    printf("StabStatus = %d\n",(int)latestData.stabStatus);
    printf("runInStatus = %d\n",(int)latestData.runInStatus);*/

    if (queue) {
        xQueueOverwrite(queue, &latestData);
    }
}

//get queue handle from bme690task
void Bme690::setOutputQueue(QueueHandle_t q)
{
    queue = q;
}