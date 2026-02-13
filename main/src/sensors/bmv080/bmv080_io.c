/**
 * Copyright (C) BlackIoT Sagl. All Rights Reserved. Confidential.
 *
 *
 */

#include "bmv080_io.h"
#include <unistd.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
#include "driver/i2c_master.h"
#endif

/* SPI GPIO Configuration */
#if defined(CONFIG_IDF_TARGET_ESP32s3)

#ifdef __USING_SPI3__
#define PIN_NUM_MISO        37
#define PIN_NUM_MOSI        35
#define PIN_NUM_CLK         36
#define PIN_NUM_CS          34
#define SPI_CLK_FREQ        ((int)(1e6))
#define SPI_MODULE          SPI3_HOST
#else
#define PIN_NUM_MISO        13
#define PIN_NUM_MOSI        11
#define PIN_NUM_CLK         12
#define PIN_NUM_CS          10
#define SPI_CLK_FREQ        ((int)(1e6))
#define SPI_MODULE          SPI2_HOST
#endif

#else

#define PIN_NUM_MISO        13
#define PIN_NUM_MOSI        11
#define PIN_NUM_CLK         12
#define PIN_NUM_CS          10
#define SPI_CLK_FREQ        ((int)(1e6))
#define SPI_MODULE          SPI2_HOST

#endif



esp_err_t spi_init(spi_device_handle_t *spi)
{
    esp_err_t err = ESP_OK;

    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1};

    err = spi_bus_initialize(SPI_MODULE, &buscfg, SPI_DMA_CH_AUTO);

    spi_device_interface_config_t devcfg = {
        .address_bits = 16,
        .clock_speed_hz = SPI_CLK_FREQ,
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 1,
        .flags = 0 }; //SPI_DEVICE_HALFDUPLEX};

    err = spi_bus_add_device(SPI_MODULE, &devcfg, spi);
    return err;
}


esp_err_t spi_deinit(spi_device_handle_t *spi)
{
    spi_bus_remove_device(*spi);
    *spi = NULL;
    return ESP_OK;
}


int8_t bmv080_spi_read_16bit(bmv080_sercom_handle_t handle, uint16_t header, uint16_t *payload, uint16_t payload_length)
{
    spi_transaction_ext_t spi_transaction = (spi_transaction_ext_t){
        .base = {
            .flags = (SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_CMD),
            .addr = header,
            .length = payload_length * 2 * 8,
            .rxlength = payload_length * 2 * 8,
            .tx_buffer = NULL,
            .rx_buffer = (void *)payload},
        .command_bits = 0,
        .address_bits = 16,
        .dummy_bits = 0,
    };

    esp_err_t err = spi_device_polling_transmit((spi_device_handle_t)handle, (spi_transaction_t *)&spi_transaction);

    /* Conversion of payload from big endian to little endian */
    for (int payload_idx = 0; payload_idx < payload_length; payload_idx++)
    {
        uint16_t swapped_word = ((payload[payload_idx] << 8) | (payload[payload_idx] >> 8)) & 0xffff;
        payload[payload_idx] = swapped_word;
    }

    return (int8_t)err;
}


int8_t bmv080_spi_write_16bit(bmv080_sercom_handle_t handle, uint16_t header, const uint16_t *payload, uint16_t payload_length)
{
    esp_err_t err;

    /* Conversion of payload from little endian to big endian (dynamic allocation is used) */
    uint16_t *payload_swapped = (uint16_t *)calloc(payload_length, sizeof(uint16_t));
    for (int payload_idx = 0; payload_idx < payload_length; payload_idx++)
    {
        payload_swapped[payload_idx] = ((payload[payload_idx] << 8) | (payload[payload_idx] >> 8)) & 0xffff;
    }

    spi_transaction_ext_t spi_transaction = (spi_transaction_ext_t){
        .base = {
            .flags = (SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_CMD),
            .addr = header,
            .length = payload_length * 2 * 8,
            .rx_buffer = NULL,
            .tx_buffer = (void *)payload_swapped},
        .command_bits = 0,
        .address_bits = 16,
        .dummy_bits = 0,
    };

    err = spi_device_transmit((spi_device_handle_t)handle, (spi_transaction_t *)&spi_transaction);

    free(payload_swapped);

    return (int8_t)err;
}



int8_t bmv080_delay(uint32_t period)
{
    vTaskDelay(period / portTICK_PERIOD_MS);
    return ESP_OK;
}

#if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
/*
 * I2C implementations for ESP32C3/C6.
 * The sercom `handle` is an i2c_master_dev_handle_t created externally (shared bus).
 */
int8_t bmv080_i2c_read_16bit(bmv080_sercom_handle_t handle, uint16_t header, uint16_t *payload, uint16_t payload_length)
{
    if (handle == NULL || payload == NULL || payload_length == 0) {
        return -1;
    }

    i2c_master_dev_handle_t dev = (i2c_master_dev_handle_t)handle;

    // Header: 16-bit MSB-first
    uint8_t hdr[2] = { (uint8_t)((header >> 8) & 0xFF), (uint8_t)(header & 0xFF) };
    size_t rx_len = payload_length * 2;
    uint8_t *rx = (uint8_t *)malloc(rx_len);
    if (!rx) {
        return -1;
    }

    esp_err_t err = i2c_master_transmit_receive(dev, hdr, sizeof(hdr), rx, rx_len, pdMS_TO_TICKS(100));
    if (err != ESP_OK) {
        free(rx);
        return (int8_t)err;
    }

    // Convert big-endian bytes to host-order 16-bit words
    for (uint16_t i = 0; i < payload_length; ++i) {
        uint16_t be = ((uint16_t)rx[2 * i] << 8) | (uint16_t)rx[2 * i + 1];
        // Driver expects little-endian words
        uint16_t le = (uint16_t)(((be & 0x00FF) << 8) | ((be & 0xFF00) >> 8));
        payload[i] = le;
    }

    free(rx);
    return 0;
}

int8_t bmv080_i2c_write_16bit(bmv080_sercom_handle_t handle, uint16_t header, const uint16_t *payload, uint16_t payload_length)
{
    if (handle == NULL || (payload_length > 0 && payload == NULL)) {
        return -1;
    }

    i2c_master_dev_handle_t dev = (i2c_master_dev_handle_t)handle;

    size_t tx_len = 2 + payload_length * 2;
    uint8_t *tx = (uint8_t *)malloc(tx_len);
    if (!tx) {
        return -1;
    }

    // Header: 16-bit MSB-first
    tx[0] = (uint8_t)((header >> 8) & 0xFF);
    tx[1] = (uint8_t)(header & 0xFF);

    // Payload: convert from little-endian words to big-endian bytes
    for (uint16_t i = 0; i < payload_length; ++i) {
        uint16_t le = payload[i];
        uint16_t be = (uint16_t)(((le & 0x00FF) << 8) | ((le & 0xFF00) >> 8));
        tx[2 + 2 * i] = (uint8_t)((be >> 8) & 0xFF);
        tx[2 + 2 * i + 1] = (uint8_t)(be & 0xFF);
    }

    esp_err_t err = i2c_master_transmit(dev, tx, tx_len, pdMS_TO_TICKS(100));
    free(tx);
    return (int8_t)err;
}
#endif
