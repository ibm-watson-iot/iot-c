# ManagedDevice SDK

!!! note
    This page is still under construction


The managed device SDK includes a shared library that exposes functions to build device client that 
runs on IoT devices.

- `libwiotp-sdk-managedDevice-as.so`

Support APIs for creating a device client and managing connection to the Watson IoT Platform service:

- `IoTPConfig_create()`
- `IoTPManagedDevice_create()`
- `IoTPManagedDevice_connect()`

Support APIs for working with events and commands: 

- `IoTPManagedDevice_sendEvent()`
- `IoTPManagedDevice_setEventCallback()`
- `IoTPManagedDevice_setCommandsHandler()`
- `IoTPManagedDevice_subscribeToCommands()`


## Configuration

Managed device configuration is passed to the client via the `IoTPConfig` object when you create 
the client instance or handle `IoTPManagedDevice`. See the [configure managed device](config.md) section 
for full details of all available options to configure a client instance.

The config handle `IoTPConfig` created using `IoTPConfig_create()` must be cleared 
using `IoTPConfig_clear()` to avoid handle leak. The managed device handle `IoTPManagedDevice` created 
using `IoTPManagedDevice_create()` must be destroyed using `IoTPManagedDevice_destory()` to avoid handle leak.

