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

#if !defined(IOTP_MANAGEDGATEWAY_H_)
#define IOTP_MANAGEDGATEWAY_H_

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


/*! \page iotpmanagedgateway Managed Gateway Development

   This library exposes functions and declarations to build IoTP managed gateway client.

      ** This page is under construction **

 */

/**
 * A handle representing a device client of IBM Watson IoT Platform service.
 * A valid client handle is available following a successful 
 * call to IoTPManagedGateway_create() API.
 */
typedef void * IoTPManagedGateway;


/**
 * The IoTPManagedGateway_create() API creates a device client that can connect to
 * IBM Watson IoT Platform service. The device client is created using configuration
 * specified by IoTP Config object.
 *
 * @param managedGateway - A pointer in which to return the newly created IoTPManagedGateway handle.
 * @param config         - A pointer to IoTP config handle.
 * @return IOTPRC         - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 * @remark Use IoTPManagedGateway_destroy() API, to destroy a client created using this API.
 */
DLLExport IOTPRC IoTPManagedGateway_create(IoTPManagedGateway **managedGateway, IoTPConfig *config);

/**
 * The IoTPManagedGateway_setMQTTLogHandler() API sets a callback to handle Paho MQTT client 
 * library log and trace  messages.
 *
 * @param managedGateway - A pointer to IoTP managed device handle.
 * @param cb             - Pointer to log handler
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_setMQTTLogHandler(IoTPManagedGateway *managedGateway, IoTPLogHandler *cb);

/**
 * The IoTPManagedGateway_destroy() API destroys the device client handle,
 * previously created by using IoTPManagedGateway_create() API. 
 *
 * @param managedGateway - A pointer to IoTP managed device handle.
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_destroy(IoTPManagedGateway *managedGateway);

/**
 * The IoTPManagedGateway_connect() API connects a device client to IBM Watson IoT Platform service.
 *
 * @param managedGateway - A pointer to IoTP managed device handle.
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_connect(IoTPManagedGateway *managedGateway);

/**
 * The IoTPManagedGateway_disconnect() API disconnects the device client from IBM Watson IoT Platform service.
 *
 * @param managedGateway - A pointer to IoTP managed device handle.
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_disconnect(IoTPManagedGateway *managedGateway);

/**
 * The IoTPManagedGateway_setCommandHandler() API sets the Command Callback function,
 * to receive the commands from IBM Watson IoT Platform servce.
 *
 * @param managedGateway - A pointer to IoTP managed device handle.
 * @param cb             - A Function pointer to the IoTPCallbackHandler. 
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_setCommandHandler(IoTPManagedGateway *managedGateway, IoTPCallbackHandler cb);

/**
 * The IoTPManagedGateway_subscribeToCommands() API subscribe to commands for the device.
 * To receive command specified by "commandId" paramete, a command callback handler
 * must be registered using IoTPManagedGateway_setCommandHandler() API.
 *
 * @param managedGateway - A pointer to IoTP managed device handle.
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 */
DLLExport IOTPRC IoTPManagedGateway_subscribeToCommands(IoTPManagedGateway *managedGateway, char *commandId, char *formatString);

/**
 * The IoTPManagedGateway_handleCommand() API subscribes to a specific command and 
 * sets a command handler for the command. 
 *
 * @param managedGateway - A pointer to IoTP managed device handle.
 * @param cb             - A Function pointer to the IoTPCallbackHandler. 
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_handleCommand(IoTPManagedGateway *managedGateway, IoTPCallbackHandler cb, char *commandId, char *formatString);

/**
 * The IoTPManagedGateway_unsubscribeFromCommands() API unsubscribes from commands for the device.
 *
 * @param managedGateway - A pointer to IoTP managed device handle.
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_unsubscribeFromCommands(IoTPManagedGateway *managedGateway, char *commandId, char *formatString);

/**
 * The IoTPManagedGateway_subscribeToDeviceCommands() API subscribes commands on behalf of devices
 * registered with the IBM Watson IoT Platform service via this gateway.
 * To receive command specified by "commandId" paramete, a command callback handler
 * must be registered using IoTPManagedGateway_setCommandHandler() API.
 *
 * @param managedGateway - A pointer to IoTP managed gateway handle.
 * @param typeId         - Device type ID
 * @param deviceId       - Device ID
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 * @return IOTPRC        - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_subscribeToDeviceCommands(IoTPManagedGateway *managedGateway, char *typeId, char *deviceId, char *commandId, char *formatString);

/**
 * The IoTPManagedGateway_unsubscribeFromDeviceCommands() API unsubscribes from commands
 * on behalf of devices registered with the IBM Watson IoT Platform service via this gateway.
 *
 * @param managedGateway - A pointer to IoTP managed gateway handle.
 * @param typeId         - Device type ID
 * @param deviceId       - Device ID
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 * @return IOTPRC        - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_unsubscribeFromDeviceCommands(IoTPManagedGateway *managedGateway, char *typeId, char *deviceId, char *commandId, char *formatString);


/**
 * The IoTPManagedGateway_sendEvent() API sends events from the device to the IBM Watson IoT service.
 *
 * @param managedGateway - A pointer to IoTP managed device handle.
 * @param eventId        - Event id to be published e.g status, gps
 * @param data           - Payload of the event
 * @param formatString   - Format of the event e.g json
 * @param qos            - QoS for the publish event. Supported values : QoS0, QoS1, QoS2
 * @param props          - MQTT V5 properties
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_sendEvent(IoTPManagedGateway *managedGateway, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props);

/**
 * The IoTPManagedGateway_sendDeviceEvent() API sends events on behalf of a deviec to the IBM Watson IoT Platform service.
 *
 * @param managedGateway - A pointer to IoTP managed gateway handle.
 * @param typeId         - Device type ID
 * @param deviceId       - Device ID
 * @param eventId        - Event id to be published e.g status, gps
 * @param data           - Payload of the event
 * @param formatString   - Format of the event e.g json
 * @param qos            - QoS for the publish event. Supported values : QoS0, QoS1, QoS2
 * @param props          - MQTT V5 properties
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_sendDeviceEvent(IoTPManagedGateway *managedGateway, char *typeId, char *deviceId, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props);

/**
 * The IoTPManagedGateway_setNotificationHandler() API sets a notification callback function,
 * to receive the notifications from IBM Watson IoT Platform service.
 *
 * @param managedGateway - A pointer to IoTP managedGateway handle.
 * @param cb             - A Function pointer to the IoTPCallbackHandler.
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_setNotificationHandler(IoTPManagedGateway *managedGateway, IoTPCallbackHandler cb);

/**
 * The IoTPManagedGateway_subscribeToNotifications() subscribes to managedGateway notifications.
 * To receive the notification, a notification callback function must be registered
 * using IoTPManagedGateway_setNotificationHandler() API.
 *
 * @param managedGateway - A pointer to IoTP managedGateway handle.
 * @param typeId         - Device type ID
 * @param deviceId       - Device ID
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_subscribeToNotifications(IoTPManagedGateway *managedGateway, char *typeId, char *deviceId);

/**
 * The IoTPManagedGateway_unsubscribeFromNotifications() API unsubscribes from managedGateway notifications,
 * previously subscribed to, using IoTPManagedGateway_subscribeToNotifications() API.
 *
 * @param managedGateway - A pointer to IoTP managedGateway handle.
 * @param typeId         - Device type ID
 * @param deviceId       - Device ID
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_unsubscribeFromNotifications(IoTPManagedGateway *managedGateway, char *typeId, char *deviceId);

/**
 * The IoTPManagedGateway_setMonitoringMessageHandler() API sets a monitoring messages callback function,
 * to receive the monitoring messages from IBM Watson IoT Platform service.
 *
 * @param managedGateway - A pointer to IoTP managedGateway handle.
 * @param cb             - A Function pointer to the IoTPCallbackHandler.
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_setMonitoringMessageHandler(IoTPManagedGateway *managedGateway, IoTPCallbackHandler cb);

/**
 * The IoTPManagedGateway_subscribeToMonitoringMessages() API subscribes to monitoring messages.
 * To receive the monitoring messages, a monitoring message callback function must be registered
 * using IoTPManagedGateway_setMonitoringMessageHandler() API.
 *
 * @param managedGateway - A pointer to IoTP managedGateway handle.
 * @param typeId         - Device type ID
 * @param deviceId       - Device ID
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_subscribeToMonitoringMessages(IoTPManagedGateway *managedGateway, char *typeId, char *deviceId);

/**
 * The IoTPManagedGateway_unsubscribeFromMonitoringMessages() API unsubscribes from monitoring messages,
 * previously subscribed to, using IoTPManagedGateway_subscribeToMonitoringMessages() API.
 *
 * @param managedGateway - A pointer to IoTP managedGateway handle.
 * @param typeId         - Device type ID
 * @param deviceId       - Device ID
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_unsubscribeFromMonitoringMessages(IoTPManagedGateway *managedGateway, char *typeId, char *deviceId);



/**
 * The IoTPManagedGateway_setAttribute() API sets attributes of a managed device, to be sent to 
 * the IBM Watson IoT Platform service with manage request or as an update.
 *
 * @param managedGateway - A pointer to IoTP managed device handle.
 * @param name           - Name of the attribure
 * @param value          - Value (a JSON string) of the attribute
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_setAttribute(IoTPManagedGateway *managedGateway, char *name, char *value);

/**
 * The IoTPManagedGateway_manage() API sends a manage request to IBM Watson IoT Platform service.
 *
 * @param managedGateway - A pointer to IoTP managed device handle.
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_manage(IoTPManagedGateway *managedGateway);

/**
 * The IoTPManagedGateway_unmanage() API sends an unmanage request to IBM Watson IoT Platform service.
 *
 * @param managedGateway - A pointer to IoTP managed device handle.
 * @param reqId          - Request ID
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_unmanage(IoTPManagedGateway *managedGateway, char * reqId);

/**
 * The IoTPManagedGateway_setActionDMHandler() API sets an action callback handler to process
 * device management actions (like reboot, factory_reset, firmware download, and firmware update)
 * initiated by IBM IoT Watson Platform.
 *
 * @param managedGateway - A pointer to IoTP managed device handle.
 * @param type           - Action type defined by IoTP_DMAction_type_t enum
 * @param cb             - Pointer to IoTPDMActionHandler callback handler 
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_setActionHandler(IoTPManagedGateway *managedGateway, IoTP_DMAction_type_t type, IoTPDMActionHandler cb);

/**
 * The IoTPManagedGateway_unsetActionHandler() API unsets an action callback handler previously 
 * set by using IoTPManagedGateway_setActionHandler() API.
 *
 * @param managedGateway - A pointer to IoTP managed device handle.
 * @param type           - Action type. Valid types are reboot, reset, download, update
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_unsetActionHandler(IoTPManagedGateway *managedGateway, char *type);

/**
 * The IoTPManagedGateway_actionResponse() API sends a reponse to an action  initiated 
 * by IBM IoT Watson IoT Platform.
 *
 * @param managedGateway - A pointer to IoTP managed device handle.
 * @param reqId          - Request Id associated with the action
 * @param rc             - Return code for the completed action
 * @param message        - Message to be returned to WIoTP
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_actionResponse(IoTPManagedGateway *managedGateway, char *reqId, int rc, char *message);

/**
 * The IoTPManagedGateway_addErrorCode() API to notify IBM IoT Watson IoT Platform about changes to their
 * error status.
 *
 * @param managedGateway - A pointer to IoTP managed device handle.
 * @param reqId          - Request Id associated with the action
 * @param errorCode      - Error code
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_addErrorCode(IoTPManagedGateway *managedGateway, char *reqId, int errorCode);

/**
 * The IoTPManagedGateway_clearErrorCodes() API to notify IBM IoT Watson IoT Platform to clear all error codes
 * for the device.
 *
 * @param managedGateway - A pointer to IoTP managed device handle.
 * @param reqId          - Request Id associated with the action
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_clearErrorCodes(IoTPManagedGateway *managedGateway, char *reqId);

/**
 * The IoTPManagedGateway_addLogEntry() API sends a log message to IBM Watson IoT Platform.
 *
 * @param managedGateway - A pointer to IoTP managed device handle.
 * @param reqId          - Request Id associated with the action
 * @param message        - Log message
 * @param timestamp      - Time stamp in ISO8601 format
 * @param data           - Data
 * @param severity       - Severity (0:Info, 1:Warning, 2:Error)
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_addLogEntry(IoTPManagedGateway *managedGateway, char *reqId, char *message, char *timestamp, char *data, int severity);

/**
 * The IoTPManagedGateway_clearLog() API notifies IBM Watson IoT Platform to clear diagnostic log messages.
 *
 * @param managedGateway - A pointer to IoTP managed device handle.
 * @param reqId          - Request Id associated with the action
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPManagedGateway_clearLog(IoTPManagedGateway *managedGateway, char *reqId);



#if defined(__cplusplus)
 }
#endif

#endif /* IOTP_MANAGEDGATEWAY_H_ */

