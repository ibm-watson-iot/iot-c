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

#if !defined(IOTP_GATEWAY_H_)
#define IOTP_GATEWAY_H_

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


/*! \page iotpgateway Gateway Development

IoTP C Client SDK includes a shared library `libiotp-as-gateway.so` that exposes functions
to build gateway client that runs on IoT gateways. The gateway client connects to the IBM Watson™ 
IoT Platform (WIoTP) service, and sends events to WIoTP service or receive commands from WIoTP 
service. A gateway client can also send events or receive commands on behalf of a device.

The following sections describe on how to configure, connect, send events,
receive commands, disconnect and cleanup, a gateway client.

### Configure Gateway
To use the gateway client, you need to create a gateway handle or instance of this client. 
Use the following steps to create a gateway handle:

- Include `iot_gateway.h` header file in the gateway client code.
- Create a gateway configuration object {@link IoTPConfig}, using one of the following options:
  1. By reading the configuration items from a file using {@link IoTPConfig_create()}.
  2. From environment variables using {@link IoTPConfig_readEnvironment()}.
  3. Setting configuration items using {@link IoTPConfig_setProperty()}.

  For details on configuration items and options, refer to [Configuration](./iotpconfig.html).
  If `IoTPConfig` is created successfully, on exit you must call {@link IoTPConfig_clear()} to clear
  configuration object.
- Create a gateway handle. A valid gateway handle {@link IoTPGateway}, is available following
  a successful call to {@link IoTPGateway_create()}. If `IoTPGateway` handle is created successfully,
  on exit you must call {@link IoTPGateway_destroy()} to destroy the handle.


\note Though there is no restrictions on how many gateway client handles, a gateway application can create,
it is a good practice to not to create many client handles to limit the number of connections to 
WIoTP service, and reduce load on the WIoTP service.


Sample code to configure a gateway client:
\code
#include <iotp_gateway.h>
int main(int argc, char *argv[])
{
    IOTPRC rc = IOTP_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPGateway *gateway = NULL;
    // Create IoTPConfig object using configuration options defined in the configuration file.
    rc = IoTPConfig_create(&config, "configFilePath");
    if ( rc != IOTPRC_SUCCESS ) {
        exit(1);
    }
    // Create IoTPGateway object
    rc = IoTPGateway_create(&gateway, config);
    if ( rc != IOTPRC_SUCCESS ) {
        IoTPConfig_clear(config);
        exit(1);
    }

    // Add code to connect, send event or receive commands

    // On exit call cleanup APIs
    IoTPGateway_destroy(gateway);
    IoTPConfig_clear(config);
    return 0;
}
\endcode


### Connect Gateway
You can connect a gateway clienti to WIoTP service using {@link IoTPGateway_connect()}.
This will allow the gateway client to send events, or receive and process commands.
If gateway client is connected to WIoTP service successfully, on exit you must call 
{@link IoTPGateway_disconnect()} to disconnect the gateway client.

Sample code to connect a gateway client:
\code
#include <iotp_gateway.h>
int main(int argc, char *argv[])
{
    IOTPRC rc = IOTPRC_SUCCESS;

    // Add code to configure the gateway client

    // Connect gateway
    rc = IoTPGateway_connect(gateway);
    if ( rc != IOTPRC_SUCCESS ) {
        // User defined code to handle connection failures, for example wait
        // for some time and retry connection. Or on exit invoke cleanup APIs.
    }

    // Add code to send event or receive commands

    // On exit call cleanup APIs
    IoTPGateway_disconnect(gateway);
    IoTPGateway_destroy(gateway);
    IoTPConfig_clear(config);
    return 0;
}
\endcode


### Send Event
Gateway can provide various types of information, for example data collected by the gateway 
(telemtry data), current status of the gateway etc. Events are the mechanism by which gateways 
publish data to WIoTP service. Gateways control the content of their messages, and assign a name 
for each event that is sent. WIoTP service uses the credentials that are attached to each event 
received to determine which gateway sent the event. This architecture prevents gateways from 
impersonating one another.

Events can be published with any of the three quality of service (QoS) levels that are defined 
by the MQTT protocol. By default, events are published with a QoS level of 0. Optonally you can
send MQTT V5 properties. For details on MQTT V5 properties, refer to <a href="https://www.eclipse.org/paho/files/mqttdoc/MQTTAsync/html/struct_m_q_t_t_properties.html" target="_blank"}MQTTProperties</a>.

To send an event to WIoTP service, you can use {@link IoTPGateway_sendEvent()}.

Sample code to send event (in JSON format) and no MQTT V5 properties:
\code
#include <iotp_gateway.h>
int main(int argc, char *argv[])
{
    IOTPRC rc = IOTPRC_SUCCESS;

    // Add code to configure and connect the gateway client

    // Sample event data in json format
    char *data = "{\"d\" : {\"SensorID\": \"Test\", \"Reading\": 7 }}";
    // Send status event with QoS0 and no MQTT properties
    rc = IoTPGateway_sendEvent(gateway,"status", data, "json", QoS0, NULL);
    if ( rc != IOTPRC_SUCCESS ) {
        // User defined code to handle send failures. Or on exit invoke cleanup APIs.
    }

    // Add code to receive and process commands, if required

    // On exit call cleanup APIs
    IoTPGateway_disconnect(gateway);
    IoTPGateway_destroy(gateway);
    IoTPConfig_clear(config);
    return 0;
}
\endcode


Sample code to send event (in JSON format) and MQTT V5 properties:
\code
#include <iotp_gateway.h>
int main(int argc, char *argv[])
{
    IOTPRC rc = IOTPRC_SUCCESS;

    // Add code to configure and connect the gateway client

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
    rc = IoTPGateway_sendEvent(gateway,"status", data, "json", QoS0, properties);
    if ( rc != IOTPRC_SUCCESS ) {
        // User defined code to handle failure. Or on exit invoke cleanup APIs.
    }
    // Free MQTT V5 properties
    MQTTProperties_free(properties);

    // Add code to receive commands and process, if required

    // On exit call cleanup APIs
    IoTPGateway_disconnect(gateway);
    IoTPGateway_destroy(gateway);
    IoTPConfig_clear(config);
    return 0;
}
\endcode

### Receive Command
Commands are the mechanism by which applications communicate with gateways. 
Only applications can send commands, and the commands are sent to specific gateways. 
The gateway must determine which action to take on receipt of any given command.
The gateway client must register a callback function to process the received command.

There are two ways to register a callback and subscribe to commands:

1.  A gateway client can subscribe to commands using {@link IoTPGateway_subscribeToCommands()}.
To process received commands, you must register a command callback. Use
{@link IoTPGateway_setCommandHandler()} to set command callback or handler for all commands.

2. You can also use {@link IoTPGateway_handleCommand()} to register a command callback and 
subscribe to a specific command.


Sample code to receive commands by a gateway client:
\code
#include <iotp_gateway.h>
void  gatewayCommandCallback (char* type, char* id, char* commandName, char *format, void* payload, size_t payloadSize)
{
    fprintf(stdout, "Received gateway command:\n");
    fprintf(stdout, "Type=%s ID=%s CommandName=%s Format=%s Len=%d\n",
        type?type:"", id?id:"", commandName?commandName:"", format?format:"", (int)payloadSize);
    fprintf(stdout, "Payload: %s\n", payload?(char *)payload:"");

    // Gateway developers - add your custom code to process gateway commands
}
int main(int argc, char *argv[])
{
    IOTPRC rc = IOTP_SUCCESS;

    // Add code to configure and connect the gateway client

    rc = IoTPGateway_setCommandHandler(gateway, gatewayCommandCallback);
    if ( rc != IOTPRC_SUCCESS ) {
        // User defined code to handle failure, or call APIs to cleanup and exit
    }
    char *commandName = "+"; // use wildcard subscription
    char *format = "+";      // use wildcard to receive command in any format. e.g. json, text etc
    rc = IoTPGateway_subscribeToCommands(gateway, commandName, format);
    if ( rc != IOTPRC_SUCCESS ) {
        // User defined code to handle failure. Or exit invoke cleanup APIs.
    }

    // Add code to send events if required. Refer to "Send Event" section for details.

    // Add calls to cleanup APIs. Refer to "Disconnect and Cleanup" section for details.

    // On exit call cleanup APIs 
    IoTPGateway_disconnect(gateway);
    IoTPGateway_destroy(gateway);
    IoTPConfig_clear(config);
    return 0;
}
\endcode

*/


/**
 * A handle representing gateway client of IBM Watson IoT Platform service.
 * A valid client handle is available following a successful 
 * call to IoTPGateway_create() API.
 */
typedef void * IoTPGateway;

/**
 * The IoTPGateway_create() API creates a gateway client that can connect to 
 * IBM Watson IoT Platform service. The gateway client is created using configuration
 * specified by IoTP Config object.
 *
 * @param gateway        - A pointer in which to return the newly created IoTPGateway handle.
 * @param config         - A pointer to IoTP config handle.
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 * @remark Use IoTPGateway_destroy() API, to destroy a gateway client created using this API.
 */
DLLExport IOTPRC IoTPGateway_create(IoTPGateway **gateway, IoTPConfig *config);

/**
 * The IoTPGateway_setMQTTLogHandler() API sets a callback to handle Paho MQTT client 
 * library log and trace  messages.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param cb             - Pointer to log handler
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_setMQTTLogHandler(IoTPGateway *gateway, IoTPLogHandler *cb);

/**
 * The IoTPGateway_destroy() API destroys the gateway client handle
 * previously created by using IoTPGateway_create() API.
 * 
 * @param gateway        - A pointer to IoTP gateway handle.
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_destroy(IoTPGateway *gateway);

/**
 * The IoTPGateway_connect() API connects the gateway client to IBM Watson IoT Platform service.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_connect(IoTPGateway *gateway);

/**
 * The IoTPGateway_disconnect() API disconnects the gateway client from IBM Watson IoT Platform service.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_disconnect(IoTPGateway *gateway);

/**
 * The IoTPGateway_setCommandHandler() API sets the Command Callback function,
 * to receive the commands from IBM Watson IoT Platform service.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param cb             - A Function pointer to the IoTPCallbackHandler. 
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_setCommandHandler(IoTPGateway *gateway, IoTPCallbackHandler cb);

/**
 * The IoTPGateway_subscribeToCommands() API subscribes to commands for the gateway.
 * To receive command specified by "commandId" paramete, a command callback handler
 * must be registered using IoTPGateway_setCommandHandler() API.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_subscribeToCommands(IoTPGateway *gateway, char *commandId, char *formatString);

/**
 * The IoTPGateway_handleCommand() API subscribes to a specific command and 
 * sets a command handler for the command. 
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param cb             - A Function pointer to the IoTPCallbackHandler. 
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_handleCommand(IoTPGateway *gateway, IoTPCallbackHandler cb, char *commandId, char *formatString);

/**
 * The IoTPGateway_unsubscribeFromCommands() API unsubscribes from commands for the gateway.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_unsubscribeFromCommands(IoTPGateway *gateway, char *commandId, char *formatString);

/**
 * The IoTPGateway_subscribeToDeviceCommands() API subscribes commands on behalf of devices 
 * registered with the IBM Watson IoT Platform service via this gateway.
 * To receive command specified by "commandId" paramete, a command callback handler
 * must be registered using IoTPGateway_setCommandHandler() API.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param typeId         - Device type ID
 * @param deviceId       - Device ID
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_subscribeToDeviceCommands(IoTPGateway *gateway, char *typeId, char *deviceId, char *commandId, char *formatString);

/**
 * The IoTPGateway_unsubscribeFromDeviceCommands() API unsubscribes from commands 
 * on behalf of devices registered with the IBM Watson IoT Platform service via this gateway.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param typeId         - Device type ID
 * @param deviceId       - Device ID
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_unsubscribeFromDeviceCommands(IoTPGateway *gateway, char *typeId, char *deviceId, char *commandId, char *formatString);

/**
 * The IoTPGateway_sendEvent() API sends events from the gateway to the IBM Watson IoT Platform service.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param eventId        - Event id to be published e.g status, gps
 * @param data           - Payload of the event
 * @param formatString   - Format of the event e.g json
 * @param qos            - QoS for the publish event. Supported values : QoS0, QoS1, QoS2
 * @param props          - MQTT V5 properties
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_sendEvent(IoTPGateway *gateway, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props);

/**
 * The IoTPGateway_sendDeviceEvent() API sends events on behalf of a deviec to the IBM Watson IoT Platform service.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param typeId         - Device type ID
 * @param deviceId       - Device ID
 * @param eventId        - Event id to be published e.g status, gps
 * @param data           - Payload of the event
 * @param formatString   - Format of the event e.g json
 * @param qos            - QoS for the publish event. Supported values : QoS0, QoS1, QoS2
 * @param props          - MQTT V5 properties
 * @return IOTPRC       - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_sendDeviceEvent(IoTPGateway *gateway, char *typeId, char *deviceId, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props);

/**
 * The IoTPGateway_setNotificationHandler() API sets a notification callback function, 
 * to receive the notifications from IBM Watson IoT Platform service.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param cb             - A Function pointer to the IoTPCallbackHandler.
 * @return IOTPRC       - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_setNotificationHandler(IoTPGateway *gateway, IoTPCallbackHandler cb);

/**
 * The IoTPGateway_subscribeToNotifications() subscribes to gateway notifications.
 * To receive the notification, a notification callback function must be registered
 * using IoTPGateway_setNotificationHandler() API.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param typeId         - Device type ID
 * @param deviceId       - Device ID
 * @return IOTPRC       - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_subscribeToNotifications(IoTPGateway *gateway, char *typeId, char *deviceId);

/**
 * The IoTPGateway_unsubscribeFromNotifications() API unsubscribes from gateway notifications,
 * previously subscribed to, using IoTPGateway_subscribeToNotifications() API.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param typeId         - Device type ID
 * @param deviceId       - Device ID
 * @return IOTPRC       - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_unsubscribeFromNotifications(IoTPGateway *gateway, char *typeId, char *deviceId);

/**
 * The IoTPGateway_setMonitoringMessageHandler() API sets a monitoring messages callback function, 
 * to receive the monitoring messages from IBM Watson IoT Platform service.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param cb             - A Function pointer to the IoTPCallbackHandler.
 * @return IOTPRC       - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_setMonitoringMessageHandler(IoTPGateway *gateway, IoTPCallbackHandler cb);

/**
 * The IoTPGateway_subscribeToMonitoringMessages() API subscribes to monitoring messages.
 * To receive the monitoring messages, a monitoring message callback function must be registered
 * using IoTPGateway_setMonitoringMessageHandler() API.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param typeId         - Device type ID
 * @param deviceId       - Device ID
 * @return IOTPRC       - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_subscribeToMonitoringMessages(IoTPGateway *gateway, char *typeId, char *deviceId);

/**
 * The IoTPGateway_unsubscribeFromMonitoringMessages() API unsubscribes from monitoring messages,
 * previously subscribed to, using IoTPGateway_subscribeToMonitoringMessages() API.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param typeId         - Device type ID
 * @param deviceId       - Device ID
 * @return IOTPRC       - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_unsubscribeFromMonitoringMessages(IoTPGateway *gateway, char *typeId, char *deviceId);


#if defined(__cplusplus)
 }
#endif

#endif /* IOTP_GATEWAY_H_ */

