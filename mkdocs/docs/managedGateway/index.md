# Managed Gateway SDK

!!! note
    This page is still under construction


The managed gateway SDK includes a shared library that exposes functions to build device client that 
runs on IoT gateway.

- `libwiotp-sdk-managedGateway-as.so`

Support APIs for creating a device client and managing connection to the Watson IoT Platform service:

- `IoTPConfig_create()`
- `IoTPManagedGateway_create()`
- `IoTPManagedGateway_connect()`

Support APIs for working with events and commands: 

- `IoTPManagedGateway_sendEvent()`
- `IoTPManagedGateway_setEventCallback()`
- `IoTPManagedGateway_setCommandsHandler()`
- `IoTPManagedGateway_subscribeToCommands()`


## Configuration

Managed gateway configuration is passed to the client via the `IoTPConfig` object when you create 
the client instance or handle `IoTPManagedGateway`. See the [configure managed gateway](config.md) section 
for full details of all available options to configure a client instance.

The config handle `IoTPConfig` created using `IoTPConfig_create()` must be cleared 
using `IoTPConfig_clear()` to avoid handle leak. The managed gateway handle `IoTPManagedGateway` created 
using `IoTPManagedGateway_create()` must be destroyed using `IoTPManagedGateway_destory()` to avoid handle leak.

