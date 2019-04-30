# C Client Library for IBM Watson™ IoT

NOTE: This is a draft document - for document layout review.

IoTP C Client SDK provides libraries to build and develop code for integrating and connecting
devices, and applications to IBM Watson IoT Platform. This repository contains source code for
IoTP C client libraries and samples. The IoTP C client comprises five shared libraries that 
supports asynchronous mode of operations:

* libiotp-as-device.so - for device
* libiotp-as-gateway.so - for gateway
* libiotp-as-application.so - for application
* libiotp-as-managedDevice.so - for managed device
* libiotp-as-managedGateway.so - for managed gateway

## Supported Features

| Feature  | Supported?    | Description  |
|----------|:-------------:|:-------------|
| Device connectivity | Yes | Connect device(s) to Watson IoT Platform |
| Gateway connectivity | Yes | Connect gateway(s) to Watson IoT Platform |
| Application connectivity | Yes | Connect application(s) to Watson IoT Platform |
| MQTT Protocol | Yes | Enable clients to send events and receive commands using MQTT v3.1.1 or MQTT v5 |
| HTTP Protocol | No | In a future release |
| TLS Support | Yes | Enable clients to connect securely using TLS |
| Client side certificate based authentication | Yes | Connect clients using client certificate |
| Auto reconnect | Yes | Auto reconnect |
| Device Management | Yes | Connect device as managed device to Watson IoT Platform |
| Gateway Management | Yes | Connect gateway device as managed gateway to Watson IoT Platform |
| Device Management Extension | No | Device Management Extenstions. In a future release |
| Logging and Tracing | Yes | Supports custom log handler to receive client log and trace messages |

## Dependencies

* [Paho MQTT C Client](https://eclipse.org/paho/clients/c/)
* OpenSSL development package
* Doxygen to build the documentation

Note:

IoTP C client SDK build process automatically downloads and builds the dependent Paho MQTT C library.
Ensure OpenSSL development package and Doxygen (if building documentation) is installed.


## Build instructions

The provided GNU Makefile includes the following build steps:
- Download Paho MQTT C source and build
- Build IoT client libraries
- Build IoT client samples
- Build IoT client library tests and run tests
- Build Documentation

From the client library base directory run:
```
make
sudo make -C paho.mqtt.c install
sudo make install
```

This will build Paho MQTT C libraries and IoTP client libraries and install the libraries.  

To uninstall:
```
sudo make uninstall
sudo make -C paho.mqtt.c uninstall
```

To build the samples:
```
make samples
```

To build and run tests, and to get test execution summary:
```
make tests
make run-tests
make get-test-summary
```

To build the documentation:
```
make html
```

## Supported Platforms
The client libraries are built and tested on the following platforms:
- Linux
- OSX


## Client SDK API

The ``doc``directory contains the detailed API documentation.

