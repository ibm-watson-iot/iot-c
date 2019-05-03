/*******************************************************************************
 * Copyright (c) 2018-2019 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ranjan Dasgupt          - Initial API implementation
 *
 *******************************************************************************/

#if !defined(IOTP_DEVICE_H_)
#define IOTP_DEVICE_H_

#if defined(__cplusplus)
 extern "C" {
#endif

/*
/// @cond EXCLUDE
*/

#if defined(WIN32) || defined(WIN64)
  #define DLLImport __declspec(dllimport)
  #define DLLExport __declspec(dllexport)
#else
  #define DLLImport extern
  #define DLLExport __attribute__ ((visibility ("default")))
#endif

#include <MQTTProperties.h>

#include "iotp_rc.h"
#include "iotp_utils.h"
#include "iotp_config.h"

/*
/// @endcond
*/


/*! \page iotpdevice Device Development

IoTP C Client SDK includes a shared library `libiotp-as-device.so` that exposes functions
to build device client that runs on IoT devices. The device client connects to the IBM Watson™ 
IoT Platform (WIoTP) service, and sends events to WIoTP service or receive commands from WIoTP 
service.

The following sections describe on how to configure, connect, send events,
receive commands, disconnect and cleanup, a device client.

### Configure Device
To use the device client, you need to create a device handle or instance of this client. 
Use the following steps to create a device handle:

- Include `iot_device.h` header file in the device client code.
- Create a device configuration object {@link IoTPConfig}, using one of the following options:
  1. By reading the configuration items from a file using {@link IoTPConfig_create()}.
  2. From environment variables using {@link IoTPConfig_readEnvironment()}.
  3. Setting configuration items using {@link IoTPConfig_setProperty()}.

  For details on configuration items and options, refer to [Configuration](./iotpconfig.html).
  If `IoTPConfig` is created successfully, on exit you must call {@link IoTPConfig_clear()} to clear
  configuration object. 
- Create a device handle. A valid device handle {@link IoTPDevice}, is available following
  a successful call to {@link IoTPDevice_create()}. If `IoTPDevice` handle is created successfully,
  on exit you must call {@link IoTPDevice_destroy()} to destroy the handle.


\note Though there is no restrictions on how many device client handles, a device application can create,
it is a good practice to not to create many client handles to limit the number of connections to 
WIoTP service, and reduce load on the WIoTP service.


Sample code to configure a device client:
\code
#include <iotp_device.h>
int main(int argc, char *argv[])
{
    IOTPRC rc = IOTP_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPDevice *device = NULL;
    // Create IoTPConfig object using configuration options defined in the configuration file.
    rc = IoTPConfig_create(&config, "configFilePath");
    if ( rc != IOTPRC_SUCCESS ) {
        exit(1);
    }
    // Create IoTPDevice object
    rc = IoTPDevice_create(&device, config);
    if ( rc != IOTPRC_SUCCESS ) {
        IoTPConfig_clear(config);
        exit(1);
    }

    // Add code to connect, send event or receive commands

    // On exit call cleanup APIs
    IoTPDevice_destroy(device);
    IoTPConfig_clear(config);
    return 0;
}
\endcode


### Connect Device
You can connect a device clienti to WIoTP service using {@link IoTPDevice_connect()}.
This will allow the device client to send events, or receive commands.
If device client is connected to WIoTP service successfully, on exit you must call 
{@link IoTPDevice_disconnect()} to disconnect the device client.

Sample code to connect a device client:
\code
#include <iotp_device.h>
int main(int argc, char *argv[])
{
    IOTPRC rc = IOTPRC_SUCCESS;

    // Add code to configure the device client

    // Connect device
    rc = IoTPDevice_connect(device);
    if ( rc != IOTPRC_SUCCESS ) {
        // User defined code to handle connection failures, for example wait
        // for some time and retry connection. Or on exit invoke cleanup APIs.
    }

    // Add code to send event or receive commands

    // On exit call cleanup APIs
    IoTPDevice_disconnect(device);
    IoTPDevice_destroy(device);
    IoTPConfig_clear(config);
    return 0;
}
\endcode


### Send Event
Device can provide various types of information, for example data collected by the device 
(telemtry data), current status of the device etc. Events are the mechanism by which devices 
publish data to WIoTP service. Devices control the content of their messages, and assign a name 
for each event that is sent. WIoTP service uses the credentials that are attached to each event 
received to determine which device sent the event. This architecture prevents devices from 
impersonating one another.

Events can be published with any of the three quality of service (QoS) levels that are defined 
by the MQTT protocol. By default, events are published with a QoS level of 0. Optonally you can
send MQTT V5 properties. For details on MQTT V5 properties, refer to <a href="https://www.eclipse.org/paho/files/mqttdoc/MQTTAsync/html/struct_m_q_t_t_properties.html" target="_blank"}MQTTProperties</a>.

To send an event to WIoTP service, you can use {@link IoTPDevice_sendEvent()}.

Sample code to send event (in JSON format) and no MQTT V5 properties:
\code
#include <iotp_device.h>
int main(int argc, char *argv[])
{
    IOTPRC rc = IOTPRC_SUCCESS;

    // Add code to configure and connect the device client

    // Sample event data in json format
    char *data = "{\"d\" : {\"SensorID\": \"Test\", \"Reading\": 7 }}";
    // Send status event with QoS0 and no MQTT properties
    rc = IoTPDevice_sendEvent(device,"status", data, "json", QoS0, NULL);
    if ( rc != IOTPRC_SUCCESS ) {
        // User defined code to handle send failures. Or on exit invoke cleanup APIs.
    }

    // Add code to receive and process commands, if required

    // On exit call cleanup APIs
    IoTPDevice_disconnect(device);
    IoTPDevice_destroy(device);
    IoTPConfig_clear(config);
    return 0;
}
\endcode


Sample code to send event (in JSON format) and MQTT V5 properties:
\code
#include <iotp_device.h>
int main(int argc, char *argv[])
{
    IOTPRC rc = IOTPRC_SUCCESS;

    // Add code to configure and connect the device client

    // Set MQTT V5 properties
    MQTTProperties *properties = (MQTTProperties *)malloc(sizeof MQTTProperties);
    MQTTProperty   property;
    property.identifier = MQTTPROPERTY_CODE_USER_PROPERTY;
    property.value.data.data = "user defined property";
    property.value.data.len = (int)strlen(property.value.data.data);
    property.value.value.data = "user defined property value";
    property.value.value.len = (int)strlen(property.value.value.data);
    rc = MQTTProperties_add(properties, &property);
    if ( rc != IOTPRC_SUCCESS ) {
        // User defined code to handle failure. Or on exit invoke cleanup APIs.
    }
    // Sample event data in json format
    char *data = "{\"d\" : {\"SensorID\": \"Test\", \"Reading\": 7 }}";
    // Send status event with QoS0 and no MQTT properties
    rc = IoTPDevice_sendEvent(device,"status", data, "json", QoS0, properties);
    if ( rc != IOTPRC_SUCCESS ) {
        // User defined code to handle failure. Or on exit invoke cleanup APIs.
    }
    // Free MQTT V5 properties
    MQTTProperties_free(properties);

    // Add code to receive commands and process, if required

    // On exit call cleanup APIs
    IoTPDevice_disconnect(device);
    IoTPDevice_destroy(device);
    IoTPConfig_clear(config);
    return 0;
}
\endcode

\note Device can collect large amount of data. Depending on the use case, users will have to predict 
which data provides the most value. They can predict this based on impact of data collection, 
power consumption of the device, and data transmission speed and latency, how the data is consumed 
by back-end applications. Users will also have to decide on how to represent the event from devices. 
Sometimes it is difficult to decide what constitues a device. For example a commerical refrigiration
units (used in departmental stores) may have multiple bins or compartments with temperature sensors 
in each of the compartments. In this case, each compartment can be modeled as a device with its own 
unique device ID or the whole unit could be one device with temerature data from each compartment. 
So events could be modeled as different levels of abstraction. Some of the examples could be:

\note <b>Example 1:</b> Modeled as each temerature sensor as a device:
```
{"deviceId":"refUnit1_compartment1","temp":18}
{"deviceId":"refunit1_compartment2","temp":20}
```

\note <b>Example 2:</b> Modeled as refrigiration unit as a device:
```
{"deviceId":"refunit1","compartment1_temp":18, "compartment2_temp":20}
```

\note <b>Example 3:</b> Modeled as refrigiration unit as a device and temp data in an array:
```
{"deviceId":"refunit1","compartments":[{"temp":18},{"temp":20}]}
```

\note Considering the size of event, and consumability by back-end application, 
Example 3 is a better option to use compared with Example 1 and Example 2.


### Receive Command
Commands are the mechanism by which applications communicate with devices. 
Only applications can send commands, and the commands are sent to specific devices. 
The device must determine which action to take on receipt of any given command.
The device client must register a callback function to process the received command.

There are two ways to register a callback and subscribe to commands:

1.  A device client can subscribe to commands using {@link IoTPDevice_subscribeToCommands()}.
To process received commands, you must register a command callback. Use
{@link IoTPDevice_setCommandsHandler()} to set command callback or handler for all commands.

2. You can also use {@link IoTPDevice_setCommandHandler()} to register a command callback and 
subscribe to a specific command.


Sample code to receive commands by a device client:
\code
#include <iotp_device.h>
void  deviceCommandCallback (char* type, char* id, char* commandName, char *format, void* payload, size_t payloadSize)
{
    fprintf(stdout, "Received device command:\n");
    fprintf(stdout, "Type=%s ID=%s CommandName=%s Format=%s Len=%d\n",
        type?type:"", id?id:"", commandName?commandName:"", format?format:"", (int)payloadSize);
    fprintf(stdout, "Payload: %s\n", payload?(char *)payload:"");

    // Device developers - add your custom code to process device commands
}
int main(int argc, char *argv[])
{
    IOTPRC rc = IOTP_SUCCESS;

    // Add code to configure and connect the device client

    rc = IoTPDevice_setCommandsHandler(device, deviceCommandCallback);
    if ( rc != IOTPRC_SUCCESS ) {
        // User defined code to handle failure, or call APIs to cleanup and exit
    }
    char *commandName = "+"; // use wildcard subscription
    char *format = "+";      // use wildcard to receive command in any format. e.g. json, text etc
    rc = IoTPDevice_subscribeToCommands(device, commandName, format);
    if ( rc != IOTPRC_SUCCESS ) {
        // User defined code to handle failure. Or exit invoke cleanup APIs.
    }

    // Add code to send events if required. Refer to "Send Event" section for details.

    // Add calls to cleanup APIs. Refer to "Disconnect and Cleanup" section for details.

    // On exit call cleanup APIs 
    IoTPDevice_disconnect(device);
    IoTPDevice_destroy(device);
    IoTPConfig_clear(config);
    return 0;
}
\endcode



*/

/* ********************************************************************************* */


/**
 * A handle representing a device client of IBM Watson IoT Platform service.
 * A valid client handle is available following a successful 
 * call to IoTPDevice_create() API.
 */
typedef void * IoTPDevice;

/**
 * The IoTPDevice_create() API creates a device client that can connect to WIoTP service.
 * IBM Watson IoT Platform service. This API takes the following arguments:
 *
 * @param device         - A pointer in which to return the newly created IoTPDevice handle.
 * @param config         - A pointer to IoTP config handle.
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 * @remark Use IoTPDevice_destroy() API, to destroy a client created using this API.
 */
DLLExport IOTPRC IoTPDevice_create(IoTPDevice **device, IoTPConfig *config);

/**
 * The IoTPDevice_setMQTTLogHandler() API sets a callback to handle Paho MQTT client 
 * library log and trace  messages.
 *
 * @param device         - A pointer to IoTP device handle.
 * @param cb             - Pointer to log handler
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPDevice_setMQTTLogHandler(IoTPDevice *device, IoTPLogHandler *cb);

/**
 * The IoTPDevice_destroy() API destroys the device client handle,
 * previously created by using IoTPDevice_create() API. 
 *
 * @param device         - A pointer to IoTP device handle.
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPDevice_destroy(IoTPDevice *device);

/**
 * The IoTPDevice_connect() API connects a device client to IBM Watson IoT Platform service.
 *
 * @param device         - A pointer to IoTP device handle.
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPDevice_connect(IoTPDevice *device);

/**
 * The IoTPDevice_disconnect() API disconnects the device client from IBM Watson IoT Platform service.
 *
 * @param device         - A pointer to IoTP device handle.
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPDevice_disconnect(IoTPDevice *device);

/**
 * The IoTPDevice_setCommandsHandler() API sets the Command Callback function,
 * to receive the commands from IBM Watson IoT Platform servce.
 *
 * @param device         - A pointer to IoTP device handle.
 * @param cb             - A Function pointer to the IoTPCallbackHandler. 
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPDevice_setCommandsHandler(IoTPDevice *device, IoTPCallbackHandler cb);

/**
 * The IoTPDevice_subscribeToCommands() API subscribe to commands for the device.
 * To receive command specified by "commandId" paramete, a command callback handler
 * must be registered using IoTPDevice_setCommandHandler() API.
 *
 * @param device         - A pointer to IoTP device handle.
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPDevice_subscribeToCommands(IoTPDevice *device, char *commandId, char *formatString);

/**
 * The IoTPDevice_setCommandHandler() API subscribes to a specific command and 
 * sets a command handler for the command. 
 *
 * @param device         - A pointer to IoTP device handle.
 * @param cb             - A Function pointer to the IoTPCallbackHandler. 
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPDevice_setCommandHandler(IoTPDevice *device, IoTPCallbackHandler cb, char *commandId, char *formatString);

/**
 * The IoTPDevice_unsubscribeFromCommands() API unsubscribes from commands for the device.
 *
 * @param device         - A pointer to IoTP device handle.
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPDevice_unsubscribeFromCommands(IoTPDevice *device, char *commandId, char *formatString);

/**
 * The IoTPDevice_sendEvent() API sends events from the device to the IBM Watson IoT service.
 *
 * @param device         - A pointer to IoTP device handle.
 * @param eventId        - Event id to be published e.g status, gps
 * @param data           - Payload of the event
 * @param formatString   - Format of the event e.g json
 * @param qos            - QoS for the publish event. Supported values : QoS0, QoS1, QoS2
 * @param props          - MQTT V5 properties
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPDevice_sendEvent(IoTPDevice *device, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props);

#if defined(__cplusplus)
 }
#endif

#endif /* IOTP_DEVICE_H_ */

