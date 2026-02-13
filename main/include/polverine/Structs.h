#ifndef STRUCTS_H
#define STRUCTS_H

#include "freertos/FreeRTOS.h"

struct BME690Data
{
    float temperature;
    float raw_pressure;
    float humidity;
    float iaq;
    float iaq_accuracy;
    float co2;
    float breath_voc;
    float stabStatus;
    float runInStatus;
};

struct BMV080Data
{
    float runtime_sec;
    float pm_2_5_mass;
    float pm10_mass;
    float pm2_5_number;
    float pm10_number;
    float pm1;
    bool is_obstructed;
    bool is_outside_measurement_range;
};

struct Sps30Data
{
    float mc_1p0;
    float mc_2p5;
    float mc_4p0;
    float mc_10p0;

    float nc_0p5;
    float nc_1p0;
    float nc_2p5;
    float nc_4p0;
    float nc_10p0;

    float typical_particle_size;
};

struct Tc74a2Data
{
    float temperature;
};

struct CombinedSensorData
{
    BME690Data bme690;
    Sps30Data sps30;
    Tc74a2Data tc74a2;
    BMV080Data bmv080;

    // Flags to indicate which sensors have provided data
    bool bme690_valid;
    bool sps30_valid;
    bool tc74a2_valid;
    bool bmv080_valid;

    // Timestamp when data was collected
    TickType_t timestamp;
};

// LoRa transmission data types
enum data_type {
    SPS30 = 1,
    BMV080 = 2,
    BME690 = 3,
    TC74A2 = 4,
};

struct SPS30_data {
    float pm25_numerical;
    float pm25_mass; 
    float pm10_numerical;   
    float pm10_mass;
};

struct bme690_data {
    float voc;
    float pressure;
    float humidity;
};

struct temperature_data {
    float temperature;
};

struct bmv080_data {
    float pm25_numerical;
    float pm25_mass;    
    float pm10_numerical;
    float pm10_mass;
};

struct sensor_data {
    data_type type;
    union {
        struct SPS30_data sps_data;
        struct bme690_data bme_data;
        struct temperature_data t_data;
        struct bmv080_data bmv_data;
        
    } data;
};


enum setup_states {
    SET_AUTOON,
    JOIN_NETWORK,
    SETUP_DONE
};
#endif