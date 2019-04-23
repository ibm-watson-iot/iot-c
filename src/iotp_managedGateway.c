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

#include "iotp_managedGateway.h"
#include "iotp_internal.h"


/* Gateway APIs are wrapper functions of internal iotp_managed_client_*() and iotp_client_*() functions */


/* Creates a WIoTP managed gateway client */
IOTPRC IoTPManagedGateway_create(IoTPManagedGateway **managedGateway, IoTPConfig *config) 
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_create((void **)managedGateway, config, IoTPClient_managed_gateway);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to create IoTPManagedGateway: rc=%d", rc);
    }

    return rc;
}

/* Set MQTT Trace handler */
IOTPRC IoTPManagedGateway_setMQTTLogHandler(IoTPManagedGateway *managedGateway, IoTPLogHandler *cb) 
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setMQTTLogHandler((void *)managedGateway, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set MQTT Log handler: rc=%d", rc);
    }

    return rc;
}

/* Disconnects and destroys a WIoTP managed gateway client */
IOTPRC IoTPManagedGateway_destroy(IoTPManagedGateway *managedGateway)
{
    IOTPRC rc = IOTPRC_SUCCESS;
    int destroyMQTTClient = 1;

    /* disconnect and destroy client */
    rc = iotp_client_destroy((void *)managedGateway, destroyMQTTClient);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to destroy IoTPManagedGateway: rc=%d", rc);
    }

    return rc;
}

/* Connects to WIoTP */
IOTPRC IoTPManagedGateway_connect(IoTPManagedGateway *managedGateway)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_connect((void *)managedGateway);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to connect IoTPManagedGateway: rc=%d", rc);
    }

    return rc;
}

/* Disconnects from WIoTP */
IOTPRC IoTPManagedGateway_disconnect(IoTPManagedGateway *managedGateway)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_disconnect((void *)managedGateway);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to disconnect IoTPManagedGateway: rc=%d", rc);
    }

    return rc;
}

/* Sends event to WIoTP */
IOTPRC IoTPManagedGateway_sendEvent(IoTPManagedGateway *managedGateway, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !managedGateway || !eventId || *eventId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "IoTPManagedGateway_sendEvent received NULL arguments: rc=%d", rc);
        return rc;
    }
    if ( qos != QoS0 && qos != QoS1 && qos != QoS2 ) {
        rc = IOTPRC_PARAM_INVALID_VALUE;
        LOG(WARN, "IoTPManagedGateway_sendEvent received invalid arguments: rc=%d", rc);
        return rc;
    }


    /* get device type and id of this managed gateway object */
    char *typeId     = iotp_client_getDeviceType((void *)managedGateway);
    char *deviceId   = iotp_client_getDeviceId((void *)managedGateway);

    if ( deviceId == NULL || typeId == NULL ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(ERROR, "Invalid configuration. rc=%d", rc);
        return rc;
    }

    /* set topic */
    int tlen = strlen(typeId) + strlen(deviceId) + strlen(eventId) + strlen(formatString) + 26;
    char publishTopic[tlen];
    snprintf(publishTopic, tlen, "iot-2/type/%s/id/%s/evt/%s/fmt/%s", typeId, deviceId, eventId, formatString);

    LOG(DEBUG,"Send event. Topic: %s", publishTopic);

    rc = iotp_client_publish((void *)managedGateway, publishTopic, data, qos, props);

    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "IoTPManagedGateway failed to send event: %s rc=%d", eventId, rc);
    }

    return rc;
}


IOTPRC IoTPManagedGateway_sendDeviceEvent(IoTPManagedGateway *managedGateway, char *typeId, char *deviceId, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !eventId || *eventId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL arguments, rc=%d", rc);
        return rc;
    }

    char publishTopic[strlen(typeId) + strlen(deviceId) + strlen(eventId) + strlen(formatString) + 26];
    sprintf(publishTopic, "iot-2/type/%s/id/%s/evt/%s/fmt/%s", typeId, deviceId, eventId, formatString);

    LOG(DEBUG,"Calling publishData to publish to topic - %s",publishTopic);

    rc = iotp_client_publish((void *)managedGateway, publishTopic, data, qos, props);

    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to send event: %s rc=%d", eventId, rc);
    }

    return rc;
}

IOTPRC IoTPManagedGateway_setNotificationHandler(IoTPManagedGateway *managedGatway, IoTPCallbackHandler cb)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setHandler((void *)managedGatway, NULL, IoTP_Handler_Notification, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set notification handler");
    }

    return rc;
}


IOTPRC IoTPManagedGateway_subscribeToNotifications(IoTPManagedGateway *managedGatway, char *typeId, char *deviceId)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !typeId || *typeId == '\0' || !deviceId || *deviceId == '\0' ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL arguments, rc=%d", rc);
        return rc;
    }

    int tlen = strlen(typeId) + strlen(deviceId) + 23;
    char subTopic[tlen];
    snprintf(subTopic, tlen, "iot-2/type/%s/id/%s/notify", typeId, deviceId);

    rc = iotp_client_subscribe((void *)managedGatway, subTopic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe to the notification: %s rc=%d", subTopic, rc);
    }

    return rc;
}

IOTPRC IoTPManagedGateway_unsubscribeFromNotifications(IoTPManagedGateway *managedGateway, char *typeId, char *deviceId)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !typeId || *typeId == '\0' || !deviceId || *deviceId == '\0' ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL arguments, rc=%d", rc);
        return rc;
    }

    int tlen = strlen(typeId) + strlen(deviceId) + 23;
    char subTopic[tlen];
    snprintf(subTopic, tlen, "iot-2/type/%s/id/%s/notify", typeId, deviceId);

    rc = iotp_client_unsubscribe((void *)managedGateway, subTopic);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe to the notification: %s rc=%d", subTopic, rc);
    }

    return rc;
}

IOTPRC IoTPManagedGateway_setMonitoringMessageHandler(IoTPManagedGateway *managedGateway, IoTPCallbackHandler cb)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setHandler((void *)managedGateway, NULL, IoTP_Handler_MonitoringMessage, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set monitoring message handler");
    }

    return rc;
}


IOTPRC IoTPManagedGateway_subscribeToMonitoringMessages(IoTPManagedGateway *managedGateway, char *typeId, char *deviceId)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !typeId || *typeId == '\0' || !deviceId || *deviceId == '\0' ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL arguments, rc=%d", rc);
        return rc;
    }

    int tlen = strlen(typeId) + strlen(deviceId) + 20;
    char subTopic[tlen];
    snprintf(subTopic, tlen, "iot-2/type/%s/id/%s/mon", typeId, deviceId);

    rc = iotp_client_subscribe((void *)managedGateway, subTopic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe to the monitoring messages: %s rc=%d", subTopic, rc);
    }

    return rc;
}


/* Sets command handler */
IOTPRC IoTPManagedGateway_setCommandHandler(IoTPManagedGateway *managedGateway, IoTPCallbackHandler cb)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setHandler((void *)managedGateway, NULL, IoTP_Handler_Commands, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set IoTPManagedGateway global command handler: rc=%d", rc);
    }

    return rc;
}

IOTPRC IoTPManagedGateway_unsubscribeFromMonitoringMessages(IoTPManagedGateway *managedGateway, char *typeId, char *deviceId)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !typeId || *typeId == '\0' || !deviceId || *deviceId == '\0' ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL arguments, rc=%d", rc);
        return rc;
    }

    int tlen = strlen(typeId) + strlen(deviceId) + 20;
    char subTopic[tlen];
    snprintf(subTopic, tlen, "iot-2/type/%s/id/%s/mon", typeId, deviceId);

    rc = iotp_client_unsubscribe((void *)managedGateway, subTopic);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe to the monitoring messages: %s rc=%d", subTopic, rc);
    }

    return rc;
}

IOTPRC IoTPManagedGateway_subscribeToCommands(IoTPManagedGateway *managedGateway, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "IoTPManagedGateway_subscribeToCommands received invalid or NULL arguments: rc=%d", rc);
        return rc;
    }

    /* Set topic string */
    char *format = "iot-2/cmd/%s/fmt/%s";
    int len = strlen(format) + strlen(commandId) + strlen(formatString) - 3;
    char topic[len];
    snprintf(topic, len, format, commandId, formatString);

    LOG(DEBUG,"Subscribe command. topic: %s", topic);

    rc = iotp_client_subscribe((void *)managedGateway, topic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "IoTPManagedGateway failed to subscribe to the command: %s rc=%d", topic, rc);
    }

    return rc;
}

IOTPRC IoTPManagedGateway_subscribeToDeviceCommands(IoTPManagedGateway *managedGateway, char *typeId, char *deviceId, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL arguments, rc=%d", rc);
        return rc;
    }

    int tlen = strlen(typeId) + strlen(deviceId) + strlen(commandId) + strlen(formatString) + 26;
    char subTopic[tlen];
    snprintf(subTopic, tlen, "iot-2/type/%s/id/%s/cmd/%s/fmt/%s", typeId, deviceId, commandId, formatString);

    rc = iotp_client_subscribe((void *)managedGateway, subTopic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe to the command: %s rc=%d", subTopic, rc);
    }

    return rc;
}


IOTPRC IoTPManagedGateway_unsubscribeFromDeviceCommands(IoTPManagedGateway *managedGateway, char *typeId, char *deviceId, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL arguments, rc=%d", rc);
        return rc;
    }

    int tlen = strlen(typeId) + strlen(deviceId) + strlen(commandId) + strlen(formatString) + 26;
    char subTopic[tlen];
    snprintf(subTopic, tlen, "iot-2/type/%s/id/%s/cmd/%s/fmt/%s", typeId, deviceId, commandId, formatString);

    rc = iotp_client_unsubscribe((void *)managedGateway, subTopic);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe to the command: %s rc=%d", subTopic, rc);
    }

    return rc;
}


/* Set a callback handler and subscribe to a command - not allowed is global handler is set */
IOTPRC IoTPManagedGateway_handleCommand(IoTPManagedGateway *managedGateway, IoTPCallbackHandler cb, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !cb || !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "IoTPManagedGateway_handleCommand received invalid or NULL arguments: rc=%d", rc);
        return rc;
    }

    /* check for wild card character - not supported by this API */
    if ( strstr(commandId, "+")) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "IoTPManagedGateway_handleCommand does not support wild card characters: rc=%d", rc);
        return rc;
    }

    /* set handler */
    rc = iotp_client_setHandler((void *)managedGateway, commandId, IoTP_Handler_Command, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "IoTPManagedGateway failed to set command handler: rc=%d", rc);
    }

    /* Set topic string */
    char *format = "iot-2/cmd/%s/fmt/%s";
    int len = strlen(format) + strlen(commandId) + strlen(formatString) - 3;
    char topic[len];
    snprintf(topic, len, format, commandId, formatString);

    LOG(DEBUG,"Subscribe command. topic: %s", topic);

    rc = iotp_client_subscribe((void *)managedGateway, topic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "IoTPManagedGateway failed to subscribe to the command: %s rc=%d", topic, rc);
    }

    return rc;
}

/* Unsubscribe from a command */
IOTPRC IoTPManagedGateway_unsubscribeFromCommands(IoTPManagedGateway *managedGateway, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(ERROR, "IoTPManagedGateway_unsubscribeFromCommands received invalid or NULL arguments, rc=%d", rc);
        return rc;
    }

    /* Set topic string */
    char *format = "iot-2/cmd/%s/fmt/%s";
    int len = strlen(format) + strlen(commandId) + strlen(formatString) - 3;
    char topic[len];
    snprintf(topic, len, format, commandId, formatString);

    LOG(DEBUG,"Subscribe command. topic: %s", topic);

    rc = iotp_client_unsubscribe((void *)managedGateway, topic);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "IoTPManagedGateway failed to unsubscribe from the command: %s rc=%d", topic, rc);
    }

    return rc;
}

/* Set managed gateway attributes */
IOTPRC IoTPManagedGateway_setAttribute(IoTPManagedGateway *managedGateway, char *name, char *value) 
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setAttribute((void *)managedGateway, name, value);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to create IoTPManagedGateway: rc=%d", rc);
    }

    return rc;
}


/* Make gateway as a managed gateway client */
IOTPRC IoTPManagedGateway_manage(IoTPManagedGateway *managedGateway) 
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_manage((void *)managedGateway);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to create IoTPManagedGateway: rc=%d", rc);
    }

    return rc;
}


/* Make gateway as a managed gateway client */
IOTPRC IoTPManagedGateway_unmanage(IoTPManagedGateway *managedGateway, char * reqId) 
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_unmanage((void *)managedGateway, reqId);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set unmanage: rc=%d", rc);
    }

    return rc;
}


/* Sets DM Action handler */
IOTPRC IoTPManagedGateway_setActionHandler(IoTPManagedGateway *managedGateway, IoTP_DMAction_type_t type, IoTPDMActionHandler cb)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setActionHandler((void *)managedGateway, type, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set DM Action handler: rc=%d", rc);
    }

    return rc;
}


/* Notifies error code to platform */
IOTPRC IoTPManagedGateway_addErrorCode(IoTPManagedGateway *managedGateway, char *reqId, int errorCode)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !managedGateway || !reqId ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL arguments: rc=%d", rc);
        return rc;
    }

    /* Set topic string */
    char *topic = "iotgateway-1/add/diag/errorCodes";
    char message[256];
    snprintf(message, 256, "{\"d\":{\"errorCode\":%d},\"reqId\":\"%s\"}", errorCode, reqId); 

    LOG(DEBUG,"Notify errorCode: %s", message);

    rc = iotp_client_publish((void *)managedGateway, topic, message, 1, NULL);

    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to notify errorCode:%d reqId:%s rc=%d", errorCode, reqId, rc);
    }

    return rc;
}


/* Clears error code from platform */
IOTPRC IoTPManagedGateway_clearErrorCode(IoTPManagedGateway *managedGateway, char *reqId)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !managedGateway || !reqId ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL arguments: rc=%d", rc);
        return rc;
    }

    /* Set topic string */
    char *topic = "iotgateway-1/clear/diag/errorCodes";
    char message[256];
    snprintf(message, 256, "{\"reqId\":\"%s\"}", reqId); 

    LOG(DEBUG,"Clear error codes. reqId:%s", reqId);

    rc = iotp_client_publish((void *)managedGateway, topic, message, 1, NULL);

    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to clear errorCodes. reqId:%s rc=%d", reqId, rc);
    }

    return rc;
}


/* Send log message to platform */
IOTPRC IoTPManagedGateway_addLogEntry(IoTPManagedGateway *managedGateway, char *reqId, char *message, char *timestamp, char *data, int severity)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !managedGateway || !reqId ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL arguments: rc=%d", rc);
        return rc;
    }

    /* Set topic string */
    char *topic = "iotgateway-1/add/diag/log";
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

    rc = iotp_client_publish((void *)managedGateway, topic, logmsg, 1, NULL);

    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to send diagnostic log. reqId:%s rc=%d", reqId, rc);
    }

    return rc;
}


/* Notifies platform to clear diagonostics log message */
IOTPRC IoTPManagedGateway_clearLog(IoTPManagedGateway *managedGateway, char *reqId)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !managedGateway || !reqId ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL arguments: rc=%d", rc);
        return rc;
    }

    /* Set topic string */
    char *topic = "iotgateway-1/clear/diag/log";
    char message[256];
    snprintf(message, 256, "{\"reqId\":\"%s\"}", reqId); 

    LOG(DEBUG,"Clear error codes. reqId:%s", reqId);

    rc = iotp_client_publish((void *)managedGateway, topic, message, 1, NULL);

    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to clear diagnostic log. reqId:%s rc=%d", reqId, rc);
    }

    return rc;
}

