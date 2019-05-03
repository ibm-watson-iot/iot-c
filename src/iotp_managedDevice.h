/*******************************************************************************
 * Copyright (c) 2019 IBM Corp.
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

#if !defined(IOTP_MANAGEDDEVICE_H_)
#define IOTP_MANAGEDDEVICE_H_

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


/*! \page iotpmanageddevice Managed Device Development

```
This page is still under construction.
```

   This library exposes functions and declarations to build IoTP managed device client.

 */

/**
 * A handle representing a device client of IBM Watson IoT Platform service.
 * A valid client handle is available following a successful 
 * call to IoTPManagedDevice_create() API.
 */
typedef void * IoTPManagedDevice;


/**
 * The IoTPManagedDevice_create() API creates a device client that can connect to
 * IBM Watson IoT Platform service. The device client is created using configuration
 * specified by IoTP Config object.
 *
 * @param managedDevice  - A pointer in which to return the newly created IoTPManagedDevice handle.
 * @param config         - A pointer to IoTP config handle.
 * @return IOTPRC         - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 * @remark Use IoTPManagedDevice_destroy() API, to destroy a client created using this API.
 */
DLLExport IOTPRC IoTPManagedDevice_create(IoTPManagedDevice **managedDevice, IoTPConfig *config);

/**
 * The IoTPManagedDevice_setMQTTLogHandler() API sets a callback to handle Paho MQTT client 
 * library log and trace  messages.
 *
 * @param managedDevice  - A pointer to IoTP managed device handle.
 * @param cb             - Pointer to log handler
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedDevice_setMQTTLogHandler(IoTPManagedDevice *managedDevice, IoTPLogHandler *cb);

/**
 * The IoTPManagedDevice_destroy() API destroys the device client handle,
 * previously created by using IoTPManagedDevice_create() API. 
 *
 * @param managedDevice  - A pointer to IoTP managed device handle.
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedDevice_destroy(IoTPManagedDevice *managedDevice);

/**
 * The IoTPManagedDevice_connect() API connects a device client to IBM Watson IoT Platform service.
 *
 * @param managedDevice  - A pointer to IoTP managed device handle.
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedDevice_connect(IoTPManagedDevice *managedDevice);

/**
 * The IoTPManagedDevice_disconnect() API disconnects the device client from IBM Watson IoT Platform service.
 *
 * @param managedDevice  - A pointer to IoTP managed device handle.
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedDevice_disconnect(IoTPManagedDevice *managedDevice);

/**
 * The IoTPManagedDevice_setCommandHandler() API sets the Command Callback function,
 * to receive the commands from IBM Watson IoT Platform servce.
 *
 * @param managedDevice  - A pointer to IoTP managed device handle.
 * @param cb             - A Function pointer to the IoTPCallbackHandler. 
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedDevice_setCommandHandler(IoTPManagedDevice *managedDevice, IoTPCallbackHandler cb);

/**
 * The IoTPManagedDevice_subscribeToCommands() API subscribe to commands for the device.
 * To receive command specified by "commandId" paramete, a command callback handler
 * must be registered using IoTPManagedDevice_setCommandHandler() API.
 *
 * @param managedDevice  - A pointer to IoTP managed device handle.
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 */
DLLExport IOTPRC IoTPManagedDevice_subscribeToCommands(IoTPManagedDevice *managedDevice, char *commandId, char *formatString);

/**
 * The IoTPManagedDevice_handleCommand() API subscribes to a specific command and 
 * sets a command handler for the command. 
 *
 * @param managedDevice  - A pointer to IoTP managed device handle.
 * @param cb             - A Function pointer to the IoTPCallbackHandler. 
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedDevice_handleCommand(IoTPManagedDevice *managedDevice, IoTPCallbackHandler cb, char *commandId, char *formatString);

/**
 * The IoTPManagedDevice_unsubscribeFromCommands() API unsubscribes from commands for the device.
 *
 * @param managedDevice  - A pointer to IoTP managed device handle.
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedDevice_unsubscribeFromCommands(IoTPManagedDevice *managedDevice, char *commandId, char *formatString);

/**
 * The IoTPManagedDevice_sendEvent() API sends events from the device to the IBM Watson IoT service.
 *
 * @param managedDevice  - A pointer to IoTP managed device handle.
 * @param eventId        - Event id to be published e.g status, gps
 * @param data           - Payload of the event
 * @param formatString   - Format of the event e.g json
 * @param qos            - QoS for the publish event. Supported values : QoS0, QoS1, QoS2
 * @param props          - MQTT V5 properties
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedDevice_sendEvent(IoTPManagedDevice *managedDevice, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props);

/**
 * The IoTPManagedDevice_setAttribute() API sets attributes of a managed device, to be sent to 
 * the IBM Watson IoT Platform service with manage request or as an update.
 *
 * @param managedDevice  - A pointer to IoTP managed device handle.
 * @param name           - Name of the attribure
 * @param value          - Value (a JSON string) of the attribute
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedDevice_setAttribute(IoTPManagedDevice *managedDevice, char *name, char *value);

/**
 * The IoTPManagedDevice_manage() API sends a manage request to IBM Watson IoT Platform service.
 *
 * @param managedDevice  - A pointer to IoTP managed device handle.
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedDevice_manage(IoTPManagedDevice *managedDevice);

/**
 * The IoTPManagedDevice_unmanage() API sends an unmanage request to IBM Watson IoT Platform service.
 *
 * @param managedDevice  - A pointer to IoTP managed device handle.
 * @param reqId          - Request ID
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedDevice_unmanage(IoTPManagedDevice *managedDevice, char * reqId);

/**
 * The IoTPManagedDevice_setActionDMHandler() API sets an action callback handler to process
 * device management actions (like reboot, factory_reset, firmware download, and firmware update)
 * initiated by IBM IoT Watson Platform.
 *
 * @param managedDevice  - A pointer to IoTP managed device handle.
 * @param type           - Action type defined by IoTP_DMAction_type_t enum
 * @param cb             - Pointer to IoTPDMActionHandler callback handler 
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedDevice_setActionHandler(IoTPManagedDevice *managedDevice, IoTP_DMAction_type_t type, IoTPDMActionHandler cb);

/**
 * The IoTPManagedDevice_unsetActionHandler() API unsets an action callback handler previously 
 * set by using IoTPManagedDevice_setActionHandler() API.
 *
 * @param managedDevice  - A pointer to IoTP managed device handle.
 * @param type           - Action type. Valid types are reboot, reset, download, update
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedDevice_unsetActionHandler(IoTPManagedDevice *managedDevice, char *type);

/**
 * The IoTPManagedDevice_actionResponse() API sends a reponse to an action  initiated 
 * by IBM IoT Watson IoT Platform.
 *
 * @param managedDevice  - A pointer to IoTP managed device handle.
 * @param reqId          - Request Id associated with the action
 * @param rc             - Return code for the completed action
 * @param message        - Message to be returned to WIoTP
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedDevice_actionResponse(IoTPManagedDevice *managedDevice, char *reqId, int rc, char *message);

/**
 * The IoTPManagedDevice_addErrorCode() API to notify IBM IoT Watson IoT Platform about changes to their
 * error status.
 *
 * @param managedDevice  - A pointer to IoTP managed device handle.
 * @param reqId          - Request Id associated with the action
 * @param errorCode      - Error code
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedDevice_addErrorCode(IoTPManagedDevice *managedDevice, char *reqId, int errorCode);

/**
 * The IoTPManagedDevice_clearErrorCodes() API to notify IBM IoT Watson IoT Platform to clear all error codes
 * for the device.
 *
 * @param managedDevice  - A pointer to IoTP managed device handle.
 * @param reqId          - Request Id associated with the action
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedDevice_clearErrorCodes(IoTPManagedDevice *managedDevice, char *reqId);

/**
 * The IoTPManagedDevice_addLogEntry() API sends a log message to IBM Watson IoT Platform.
 *
 * @param managedDevice  - A pointer to IoTP managed device handle.
 * @param reqId          - Request Id associated with the action
 * @param message        - Log message
 * @param timestamp      - Time stamp in ISO8601 format
 * @param data           - Data
 * @param severity       - Severity (0:Info, 1:Warning, 2:Error)
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedDevice_addLogEntry(IoTPManagedDevice *managedDevice, char *reqId, char *message, char *timestamp, char *data, int severity);

/**
 * The IoTPManagedDevice_clearLog() API notifies IBM Watson IoT Platform to clear diagnostic log messages.
 *
 * @param managedDevice  - A pointer to IoTP managed device handle.
 * @param reqId          - Request Id associated with the action
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedDevice_clearLog(IoTPManagedDevice *managedDevice, char *reqId);



#if defined(__cplusplus)
 }
#endif

#endif /* IOTP_MANAGEDDEVICE_H_ */

