# ESP32 Multi-Sensor Project


**Branch Rules:**  
- Test all scripts on the target hardware.  
- Update documentation if anything changes.  
- Make small, single-purpose commits.  
- Pull from the branch regularly before committing.  
- Use pull requests to merge changes.  
- Do not push directly to `main`.

## Supported Targets

- **ESP32-C3**: I2C pins - SDA: GPIO 8, SCL: GPIO 9
- **ESP32-C6**: I2C pins - SDA: GPIO 4, SCL: GPIO 5
- **ESP32-S3**: I2C pins - SDA: GPIO 14, SCL: GPIO 21

## Prerequisites

### ESP-IDF Setup

1. Install ESP-IDF v6.0 or later:
   ```bash
   git clone -b v6.0 --recursive https://github.com/espressif/esp-idf.git
   cd esp-idf
   ./install.sh
   . ./export.sh
   ```

### i2cdev Component Installation

The SPS30 sensor requires the i2cdev component for I2C communication:

1. Clone the i2cdev component into the `components` directory:
   ```bash
   cd components
   git clone https://github.com/UncleRus/esp-idf-lib.git
   ```

2. The i2cdev library will be automatically included in the build for SPS30 sensor support.

## Project Structure

```
ISD_SENSOR_PROJECT/
├── CMakeLists.txt          # Main CMake configuration
├── sdkconfig               # ESP-IDF configuration
├── main/
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── DummyApp.hpp
│   │   ├── I2CManager.hpp
│   │   ├── sensorControl.hpp
│   │   └── sensors/
│   │       ├── Bme690Task.hpp
│   │       ├── Sps30Task.hpp
│   │       └── Tc74a2Task.hpp
│   └── src/
│       ├── I2CManager.cpp
│       ├── app/
│       │   ├── DummyApp.cpp       # Dummy application for data logging
│       │   ├── main.cpp           # Application entry point
│       │   ├── sensorControl.cpp  # Sensor control logic
│       │   └── status.cpp         # Status reporting
│       └── sensors/
│           ├── bme690/
│           │   └── Bme690Task.cpp # BME690 sensor task
│           ├── sps30/
│           │   ├── Sps30Sensor.cpp
│           │   ├── Sps30Task.cpp
│           │   └── common/         # Sensirion SPS30 library
│           └── Tc74a2/
│               └── Tc74a2Task.cpp # TC74A2 sensor task
├── components/             # ESP-IDF components
│   ├── bme690/             # BME690 sensor library
│   ├── bmv080/             # BMV080 sensor library (not used)
│   ├── bsec/               # BSEC library for BME690
│   └── esp-idf-lib/        # i2cdev library (required for SPS30)
└── build/                  # test built binary
```

## Building and Flashing

### Set Target

Choose your ESP32 target:

```bash
# For ESP32-C3
idf.py set-target esp32c3

# For ESP32-C6
idf.py set-target esp32c6

# For ESP32-S3
idf.py set-target esp32s3
```

### Build

```bash
idf.py build
```

### Flash and Monitor

```bash
idf.py flash monitor
```

## I2C Configuration

The I2C pins are automatically configured based on the selected target:

- **ESP32-C3/C6**: Uses GPIO 8 (SDA) and GPIO 9 (SCL) to avoid conflicts with flash pins
- **ESP32-S3**: Uses GPIO 14 (SDA) and GPIO 21 (SCL)

Sensor I2C addresses:
- BME690: 0x76 (default) or 0x77
- SPS30: 0x69
- TC74A2: 0x48

All sensors operate at 100kHz clock speed on the shared bus.

## Sensor Data

The project collects data from three sensors:

### BME690 (Environmental Sensor)
- Temperature (°C)
- Pressure (Pa)
- Humidity (%)
- IAQ (Indoor Air Quality)
- CO2 equivalent (ppm)

### SPS30 (Particulate Matter Sensor)
- Mass concentration (PM1.0, PM2.5, PM4.0, PM10.0 in µg/m³)
- Number concentration (0.5μm, 1.0μm, 2.5μm, 4.0μm, 10.0μm particles)
- Typical particle size (µm)

### TC74A2 (Temperature Sensor)
- Temperature (°C)

## Combined Sensor Data Structure

The `CombinedSensorData` struct aggregates data from all sensors into a single structure for efficient processing:

```cpp
struct CombinedSensorData {
    BME690Data bme690;       // BME690 sensor data
    Sps30Data sps30;         // SPS30 sensor data
    Tc74a2Data tc74a2;       // TC74A2 sensor data
    
    bool bme690_valid;       // Validity flag for BME690 data
    bool sps30_valid;        // Validity flag for SPS30 data
    bool tc74a2_valid;       // Validity flag for TC74A2 data
    
    TickType_t timestamp;    // Timestamp when data was collected
};
```

This structure allows the application to handle sensor data asynchronously and ensures data integrity by including validity flags.

## Dummy App

The DummyApp is a simple demonstration application that receives combined sensor data from the sensor control task and logs it to the serial console. It showcases how to consume the `CombinedSensorData` structure in a real application.

Key features:
- Runs as a FreeRTOS task
- Receives data via a queue
- Logs valid sensor data with timestamps
- Handles partial data (when not all sensors are available)

Data is logged every 10 seconds or when all sensors have provided valid data, whichever comes first.

## Troubleshooting

- **GPIO conflicts**: Ensure you're using the correct target-specific pins
- **Sensor not responding**: Check I2C connections, power supply (3.3V), and pull-up resistors
- **Build errors**: Verify ESP-IDF version and i2cdev component installation

## Notes

- The project uses ESP-IDF v6.0 I2C driver API; i2cdev library is required specifically for SPS30 sensor communication
- Sensors operate asynchronously with individual FreeRTOS tasks
- Combined sensor data is sent every 10 seconds or when all sensors are valid
- BMV080 sensor library is included but currently disabled in the application
- The BSEC library for BME690 sensor processing is optimized for RISC-V architecture
- All sensors share the same I2C bus on GPIO 8 (SDA) and GPIO 9 (SCL) 
