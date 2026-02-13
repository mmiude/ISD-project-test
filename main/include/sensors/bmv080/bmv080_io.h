/**
 * Copyright (C) BlackIoT Sagl All Rights Reserved. Confidential.
 *
 */
#ifndef BMV080_IO_H_
#define BMV080_IO_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* Includes ------------------------------------------------------------------*/
#include "driver/spi_master.h"
#include "bmv080.h"
#if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
#include "driver/i2c_master.h"
#endif


/* Exported functions prototypes ---------------------------------------------*/
esp_err_t spi_init(spi_device_handle_t *spi);
esp_err_t spi_deinit(spi_device_handle_t *spi);

/* SPI read and write functions */
int8_t bmv080_spi_read_16bit(bmv080_sercom_handle_t handle, uint16_t header, uint16_t *payload, uint16_t payload_length);
int8_t bmv080_spi_write_16bit(bmv080_sercom_handle_t handle, uint16_t header, const uint16_t *payload, uint16_t payload_length);

#if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
/* I2C read and write functions (C3/C6) */
int8_t bmv080_i2c_read_16bit(bmv080_sercom_handle_t handle, uint16_t header, uint16_t *payload, uint16_t payload_length);
int8_t bmv080_i2c_write_16bit(bmv080_sercom_handle_t handle, uint16_t header, const uint16_t *payload, uint16_t payload_length);
#endif

/* Delay function */
int8_t bmv080_delay(uint32_t period);


#ifdef __cplusplus
}
#endif

#endif /* BMV080_IO_H_ */
