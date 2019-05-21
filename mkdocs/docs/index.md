# C SDK

[![Build Status](https://travis-ci.org/ibm-watson-iot/iot-c.svg?branch=master)](https://travis-ci.org/ibm-watson-iot/iot-c)
[![GitHub issues](https://img.shields.io/github/issues/ibm-watson-iot/iot-c.svg)](https://github.com/ibm-watson-iot/iot-c/issues)
[![GitHub](https://img.shields.io/github/license/ibm-watson-iot/iot-c.svg)](https://github.com/ibm-watson-iot/iot-c/blob/master/LICENSE)


Documentation for this SDK can be broken down into 4 distinct areas:

- Common Topics
    - [Basic Concepts](concepts.md)
    - [MQTT Primer](mqtt.md)
    - [Error and Return codes](errors.md)
- [Application Development](application/index.md)
- [Device Development](device/index.md)
- [Gateway Development](gateway/index.md)

## Dependencies

- [Paho MQTT C Client](https://eclipse.org/paho/clients/c/)
- OpenSSL development package

## Build

Build using GNU Makefile included in this repository

```
# make
```

## Installation

Install the built libraries using make command

```
# sudo make -C paho.mqtt.c install
# sudo make install
```


## Uninstall

Uninstalling the libraries using make command

```
# sudo make uninstall
# sudo make -C paho.mqtt.c uninstall
```

