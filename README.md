[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

![Bolder Flight Systems Logo](img/logo-words_75.png) &nbsp; &nbsp; ![Arduino Logo](img/arduino_logo_75.png)

# terabee
Driver for the [Terabee TeraRanger Hub](https://www.terabee.com/shop/lidar-tof-range-finders/teraranger-tower-evo/). This library is compatible with Arduino and CMake build systems.
   * [License](LICENSE.md)
   * [Changelog](CHANGELOG.md)
   * [Contributing guide](CONTRIBUTING.md)

# Description
Range from up to 8 sensors can be sampled over UART.

# Installation

## Arduino
Use the Arduino Library Manager to install this library or clone to your Arduino/libraries folder. This library is added as:

```C++
#include "terabee.h"
```

An example Arduino executable is located in: *examples/arduino/terabee_example/terabee_example.ino*. Teensy 3.x, 4.x, and LC devices are used for testing under Arduino and this library should be compatible with other Arduino devices.

## CMake
CMake is used to build this library, which is exported as a library target called *teragee*. The header is added as:

```C++
#include "terabee.h"
```

The library can be also be compiled stand-alone using the CMake idiom of creating a *build* directory and then, from within that directory issuing:

```
cmake .. -DMCU=MK66FX1M0
make
```

This will build the library and an example executable called *terabee_example*. The example executable source file is located at *examples/cmake/terabee_example.cc*. Notice that the *cmake* command includes a define specifying the microcontroller the code is being compiled for. This is required to correctly configure the code, CPU frequency, and compile/linker options. The available MCUs are:
   * MK20DX128
   * MK20DX256
   * MK64FX512
   * MK66FX1M0
   * MKL26Z64
   * IMXRT1062_T40
   * IMXRT1062_T41
   * IMXRT1062_MMOD

These are known to work with the same packages used in Teensy products. Also switching packages is known to work well, as long as it's only a package change.

Each target also has a *_hex*, for creating the hex file to upload to the microcontroller, and an *_upload* for using the [Teensy CLI Uploader](https://www.pjrc.com/teensy/loader_cli.html) to flash the Teensy. Instructions for setting up your build environment can be found in our [build-tools repo](https://github.com/bolderflight/build-tools).

# Namespace
This library is within the namespace *bfs*.

# Terabee

## Quick example
Below is a quick example of setting up and polling the sensor data on Serial1 using Arduino notation.

```C++
#include "terabee.h"

bfs::Terabee range(&Serial1);
bfs::Terabee::RangeData data;

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  if (!range.Begin()) {
    Serial.println("Error initializing and configuring sensor");
    while (1) {}
  }
}

void loop() {
  if (range.Read(&data)) {
    for (int8_t i = 0; i < data.MAX_CH; i++) {
      Serial.print("Sensor: ");
      Serial.print(i);
      Serial.print("\tNew Data: ");
      Serial.print(data.sensor[i].updated);
      Serial.print("\tRange (m): ");
      Serial.println(data.sensor[i].range_m);
    }
  }
}
```

## RangeData and SensorData
This struct defines the range data returned from the *Terabee* object. The struct is defined as:

```C++
struct SensorData {
  bool updated;
  float range_m;
};
struct RangeData {
  static constexpr int8_t MAX_CH = 8;
  SensorData sensor[MAX_CH];
};
```

*RangeData* is a struct consisting of an array of *SensorData* objects. Each *SensorData* object gives feedback about whether that sensor was updated and the measured range, in meters.

## Methods

**Terabee()** Default constructor, requires calling the *Config* method to setup the serial port before *Begin*

**Terabee(HardwareSerial &ast;bus)** Constructor, which takes a pointer to the serial port that the Terabee is connected to.

**void Config(HardwareSerial &ast;bus)** Required to setup the serial port when using the default constructor.

**void op_mode(const OpMode mode)** Sets the operating mode. Available options are:

| Enum | Description |
| --- | --- |
| OP_MODE_TOWER (default) | Tower operating mode |
| OP_MODE_SEQ | Sequential operating mode |
| OP_MODE_SIMULTANEOUS | Simultaneous operating mode |

Please reference the manual for a detailed description of the operating mode.

**OpMode op_mode()** Returns the operating mode.

**void output_rate(const OutputRate rate)** Sets the output rate. Available options are:

| Enum | Description |
| --- | --- |
| OUTPUT_RATE_ASAP (default) | Outputs data as soon as a measurement from each sensor is available |
| OUTPUT_RATE_50HZ | OOutputs data at a rate of 50 Hz, regardless of whether each sensor is available |
| OUTPUT_RATE_100HZ | Outputs data at a rate of 100 Hz, regardless of whether each sensor is available |
| OUTPUT_RATE_250HZ | Outputs data at a rate of 250 Hz, regardless of whether each sensor is available |
| OUTPUT_RATE_500HZ | Outputs data at a rate of 500 Hz, regardless of whether each sensor is available |
| OUTPUT_RATE_600HZ | Outputs data at a rate of 600 Hz, regardless of whether each sensor is available |

Please reference the manual for a detailed description of output rates. Note that for fixed rates, the rate seems to be set for a tower with 4 sensors and half the specified rate for a tower with 8 sensors.

**OutputRate output_rate()** Returns the output rate.

**bool Begin()** The *Begin* method should be called after the serial port, operating mode, and output rate are set. Establishes communication with the sensor and returns true on successfully configuring it.

**bool Read(RangeData &ast; const data)** Returns true when new data is received. Should be passed a pointer to a *RangeData* object, which will be populated with the received sensor data.


