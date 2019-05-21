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
 *
 * Contrinutors:
 *    Ranjan Dasgupta         - Initial drop
 *
 *******************************************************************************/

#include "iotp_managedDevice.h"
#include "iotp_internal.h"


/* Device APIs are wrapper functions of internal iotp_managed_client_*() and iotp_client_*() functions */


/* Creates a WIoTP managed device client */
IOTPRC IoTPManagedDevice_create(IoTPManagedDevice **managedDevice, IoTPConfig *config) 
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_create((void **)managedDevice, config, IoTPClient_managed_device);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to create IoTPManagedDevice: rc=%d", rc);
    }

    return rc;
}

/* Set MQTT Trace handler */
IOTPRC IoTPManagedDevice_setMQTTLogHandler(IoTPManagedDevice *managedDevice, IoTPLogHandler *cb) 
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setMQTTLogHandler((void *)managedDevice, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set MQTT Log handler: rc=%d", rc);
    }

    return rc;
}

/* Disconnects and destroys a WIoTP managed device client */
IOTPRC IoTPManagedDevice_destroy(IoTPManagedDevice *managedDevice)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_destroy((void *)managedDevice);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to destroy IoTPManagedDevice: rc=%d", rc);
    }

    return rc;
}

/* Connects to WIoTP */
IOTPRC IoTPManagedDevice_connect(IoTPManagedDevice *managedDevice)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_connect((void *)managedDevice);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to connect IoTPManagedDevice: rc=%d", rc);
    }

    return rc;
}

/* Disconnects from WIoTP */
IOTPRC IoTPManagedDevice_disconnect(IoTPManagedDevice *managedDevice)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_disconnect((void *)managedDevice);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to disconnect IoTPManagedDevice: rc=%d", rc);
    }

    return rc;
}

/* Set event callback */
IOTPRC IoTPManagedDevice_setEventCallback(IoTPManagedDevice *managedDevice, IoTPEventCallbackHandler cb)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setEventCallbackHandler((void *)managedDevice, IoTP_Handler_EventCallback, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set callback for events. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Sends event to WIoTP */
IOTPRC IoTPManagedDevice_sendEvent(IoTPManagedDevice *managedDevice, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !managedDevice || !eventId || *eventId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "IoTPManagedDevice_sendEvent received NULL arguments: rc=%d", rc);
        return rc;
    }
    if ( qos != QoS0 && qos != QoS1 && qos != QoS2 ) {
        rc = IOTPRC_ARGS_INVALID_VALUE;
        LOG(WARN, "IoTPManagedDevice_sendEvent received invalid arguments: rc=%d", rc);
        return rc;
    }


    /* Set topic string */
    char *format = "iot-2/evt/%s/fmt/%s";
    int len = strlen(format) + strlen(eventId) + strlen(formatString) - 3;
    char topic[len];
    snprintf(topic, len, format, eventId, formatString);

    LOG(DEBUG,"Send event. topic: %s", topic);

    rc = iotp_client_publish((void *)managedDevice, topic, data, qos, props);

    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "IoTPManagedDevice failed to send event: %s rc=%d", eventId, rc);
    }

    return rc;
}

/* Sets command handler */
IOTPRC IoTPManagedDevice_setCommandHandler(IoTPManagedDevice *managedDevice, IoTPCallbackHandler cb)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setHandler((void *)managedDevice, NULL, IoTP_Handler_Commands, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set IoTPManagedDevice global command handler: rc=%d", rc);
    }

    return rc;
}


IOTPRC IoTPManagedDevice_subscribeToCommands(IoTPManagedDevice *managedDevice, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "IoTPManagedDevice_subscribeToCommands received invalid or NULL arguments: rc=%d", rc);
        return rc;
    }

    /* Set topic string */
    char *format = "iot-2/cmd/%s/fmt/%s";
    int len = strlen(format) + strlen(commandId) + strlen(formatString) - 3;
    char topic[len];
    snprintf(topic, len, format, commandId, formatString);

    LOG(DEBUG,"Subscribe command. topic: %s", topic);

    rc = iotp_client_subscribe((void *)managedDevice, topic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "IoTPManagedDevice failed to subscribe to the command: %s rc=%d", topic, rc);
    }

    return rc;
}

/* Set a callback handler and subscribe to a command - not allowed is global handler is set */
IOTPRC IoTPManagedDevice_handleCommand(IoTPManagedDevice *managedDevice, IoTPCallbackHandler cb, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !cb || !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "IoTPManagedDevice_handleCommand received invalid or NULL arguments: rc=%d", rc);
        return rc;
    }

    /* check for wild card character - not supported by this API */
    if ( strstr(commandId, "+")) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "IoTPManagedDevice_handleCommand does not support wild card characters: rc=%d", rc);
        return rc;
    }

    /* set handler */
    rc = iotp_client_setHandler((void *)managedDevice, commandId, IoTP_Handler_Command, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "IoTPManagedDevice failed to set command handler: rc=%d", rc);
    }

    /* Set topic string */
    char *format = "iot-2/cmd/%s/fmt/%s";
    int len = strlen(format) + strlen(commandId) + strlen(formatString) - 3;
    char topic[len];
    snprintf(topic, len, format, commandId, formatString);

    LOG(DEBUG,"Subscribe command. topic: %s", topic);

    rc = iotp_client_subscribe((void *)managedDevice, topic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "IoTPManagedDevice failed to subscribe to the command: %s rc=%d", topic, rc);
    }

    return rc;
}

/* Unsubscribe from a command */
IOTPRC IoTPManagedDevice_unsubscribeFromCommands(IoTPManagedDevice *managedDevice, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(ERROR, "IoTPManagedDevice_unsubscribeFromCommands received invalid or NULL arguments, rc=%d", rc);
        return rc;
    }

    /* Set topic string */
    char *format = "iot-2/cmd/%s/fmt/%s";
    int len = strlen(format) + strlen(commandId) + strlen(formatString) - 3;
    char topic[len];
    snprintf(topic, len, format, commandId, formatString);

    LOG(DEBUG,"Subscribe command. topic: %s", topic);

    rc = iotp_client_unsubscribe((void *)managedDevice, topic);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "IoTPManagedDevice failed to unsubscribe from the command: %s rc=%d", topic, rc);
    }

    return rc;
}

/* Set managed device attributes */
IOTPRC IoTPManagedDevice_setAttribute(IoTPManagedDevice *managedDevice, char *name, char *value) 
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setAttribute((void *)managedDevice, name, value);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to create IoTPManagedDevice: rc=%d", rc);
    }

    return rc;
}


/* Make device as a managed device client */
IOTPRC IoTPManagedDevice_manage(IoTPManagedDevice *managedDevice) 
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_manage((void *)managedDevice);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to create IoTPManagedDevice: rc=%d", rc);
    }

    return rc;
}


/* Make device as a managed device client */
IOTPRC IoTPManagedDevice_unmanage(IoTPManagedDevice *managedDevice, char * reqId) 
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_unmanage((void *)managedDevice, reqId);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set unmanage: rc=%d", rc);
    }

    return rc;
}


/* Sets DM Action handler */
IOTPRC IoTPManagedDevice_setActionHandler(IoTPManagedDevice *managedDevice, IoTP_DMAction_type_t type, IoTPDMActionHandler cb)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setActionHandler((void *)managedDevice, type, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set DM Action handler: rc=%d", rc);
    }

    return rc;
}


/* Notifies error code to platform */
IOTPRC IoTPManagedDevice_addErrorCode(IoTPManagedDevice *managedDevice, char *reqId, int errorCode)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !managedDevice || !reqId ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL arguments: rc=%d", rc);
        return rc;
    }

    /* Set topic string */
    char *topic = "iotdevice-1/add/diag/errorCodes";
    char message[256];
    snprintf(message, 256, "{\"d\":{\"errorCode\":%d},\"reqId\":\"%s\"}", errorCode, reqId); 

    LOG(DEBUG,"Notify errorCode: %s", message);

    rc = iotp_client_publish((void *)managedDevice, topic, message, 1, NULL);

    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to notify errorCode:%d reqId:%s rc=%d", errorCode, reqId, rc);
    }

    return rc;
}


/* Clears error code from platform */
IOTPRC IoTPManagedDevice_clearErrorCode(IoTPManagedDevice *managedDevice, char *reqId)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !managedDevice || !reqId ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL arguments: rc=%d", rc);
        return rc;
    }

    /* Set topic string */
    char *topic = "iotdevice-1/clear/diag/errorCodes";
    char message[256];
    snprintf(message, 256, "{\"reqId\":\"%s\"}", reqId); 

    LOG(DEBUG,"Clear error codes. reqId:%s", reqId);

    rc = iotp_client_publish((void *)managedDevice, topic, message, 1, NULL);

    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to clear errorCodes. reqId:%s rc=%d", reqId, rc);
    }

    return rc;
}


/* Send log message to platform */
IOTPRC IoTPManagedDevice_addLogEntry(IoTPManagedDevice *managedDevice, char *reqId, char *message, char *timestamp, char *data, int severity)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !managedDevice || !reqId ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL arguments: rc=%d", rc);
        return rc;
    }

    /* Set topic string */
    char *topic = "iotdevice-1/add/diag/log";
    char *logmsg = NULL;
    char *logmsgformat = "{\"d\":{\"message\":\"%s\",\"timestamp\":\"%s\",\"data\":\"%s\",\"severity\":%d},\"reqId\":\"%s\"}";
    int logmsglen = 0;
    int  msglen = 0;
    int  tslen = 0;
    int  datalen = 0;
    int reqidlen = 0;

    if ( message ) msglen = strlen(message);
    if ( timestamp ) tslen = strlen(timestamp);
    if ( data ) datalen = strlen(data);
    if ( reqId ) reqidlen = strlen(reqId);
    
    logmsglen = strlen(logmsgformat) + msglen + tslen + datalen + reqidlen + 16;
    logmsg = (char *)malloc(logmsglen);
    snprintf(logmsg, logmsglen, logmsgformat, message?message:"", timestamp?timestamp:"", data?data:"", severity, reqId);

    LOG(DEBUG,"Add log: %s", logmsg);

    rc = iotp_client_publish((void *)managedDevice, topic, logmsg, 1, NULL);

    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to send diagnostic log. reqId:%s rc=%d", reqId, rc);
    }

    return rc;
}


/* Notifies platform to clear diagonostics log message */
IOTPRC IoTPManagedDevice_clearLog(IoTPManagedDevice *managedDevice, char *reqId)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !managedDevice || !reqId ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL arguments: rc=%d", rc);
        return rc;
    }

    /* Set topic string */
    char *topic = "iotdevice-1/clear/diag/log";
    char message[256];
    snprintf(message, 256, "{\"reqId\":\"%s\"}", reqId); 

    LOG(DEBUG,"Clear error codes. reqId:%s", reqId);

    rc = iotp_client_publish((void *)managedDevice, topic, message, 1, NULL);

    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to clear diagnostic log. reqId:%s rc=%d", reqId, rc);
    }

    return rc;
}

