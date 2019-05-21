/*******************************************************************************
 * Copyright (c) 2017-2018 IBM Corp.
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

#include "iotp_gateway.h"
#include "iotp_internal.h"


/* Creates a gateway handle */
IOTPRC IoTPGateway_create(IoTPGateway **gateway, IoTPConfig *config) 
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_create((void **)gateway, config, IoTPClient_gateway);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to create device handle. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Set MQTT trace handler */
IOTPRC IoTPGateway_setMQTTLogHandler(IoTPGateway *gateway, IoTPLogHandler *cb)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setMQTTLogHandler((void *)gateway, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set MQTT Log handler. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Destroys a gateway handle */
IOTPRC IoTPGateway_destroy(IoTPGateway *gateway)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_destroy((void *)gateway);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to destroy gateway handle. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Connects to WIoTP */
IOTPRC IoTPGateway_connect(IoTPGateway *gateway)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_connect((void *)gateway);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to connect. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}


/* Disconnects from WIoTP */
IOTPRC IoTPGateway_disconnect(IoTPGateway *gateway)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_disconnect((void *)gateway);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to disconnect. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Set event callback */
IOTPRC IoTPGateway_setEventCallback(IoTPGateway *gateway, IoTPEventCallbackHandler cb)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setEventCallbackHandler((void *)gateway, IoTP_Handler_EventCallback, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set callback for events. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Sends an event */
IOTPRC IoTPGateway_sendEvent(IoTPGateway *gateway, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !gateway || !eventId || *eventId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Received NULL argument. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }
    if ( qos != QoS0 && qos != QoS1 && qos != QoS2 ) {
        rc = IOTPRC_ARGS_INVALID_VALUE;
        LOG(WARN, "Invalid QoS. qos: %d | rc: %d | reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }

    /* get device type and id of this gateway object */
    char *typeId     = iotp_client_getDeviceType((void *)gateway);
    char *deviceId   = iotp_client_getDeviceId((void *)gateway);

    if ( deviceId == NULL || typeId == NULL ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(ERROR, "NULL gateway device type or id. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }
        
    /* set topic */
    int tlen = strlen(typeId) + strlen(deviceId) + strlen(eventId) + strlen(formatString) + 26;
    char publishTopic[tlen];
    snprintf(publishTopic, tlen, "iot-2/type/%s/id/%s/evt/%s/fmt/%s", typeId, deviceId, eventId, formatString);

    LOG(DEBUG,"Send event. Topic: %s", publishTopic);

    rc = iotp_client_publish((void *)gateway, publishTopic, data, qos, props);

    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to send. event: %s | rc: %d | reason: %s", eventId, rc, IOTPRC_toString(rc));
    }

    return rc;
}


/* Sends event on behalf of a device */
IOTPRC IoTPGateway_sendDeviceEvent(IoTPGateway *gateway, char *typeId, char *deviceId, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !gateway || !typeId || *typeId == '\0'|| !deviceId || *deviceId == '\0' || !eventId || *eventId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Received NULL argument. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }
    if ( qos != QoS0 && qos != QoS1 && qos != QoS2 ) {
        rc = IOTPRC_ARGS_INVALID_VALUE;
        LOG(WARN, "Invalid QoS. qos: %d | rc: %d | reason: %s", qos, rc, IOTPRC_toString(rc));
        return rc;
    }

    char publishTopic[strlen(typeId) + strlen(deviceId) + strlen(eventId) + strlen(formatString) + 26];
    sprintf(publishTopic, "iot-2/type/%s/id/%s/evt/%s/fmt/%s", typeId, deviceId, eventId, formatString);

    LOG(DEBUG,"Send device event. topic: %s", publishTopic);

    rc = iotp_client_publish((void *)gateway, publishTopic, data, qos, props);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to send. event: %s | rc: %d | reason: %s", eventId, rc, IOTPRC_toString(rc));
    }

    return rc;
}


/* Sets a handler for all commands */
IOTPRC IoTPGateway_setCommandHandler(IoTPGateway *gateway, IoTPCallbackHandler cb)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setHandler((void *)gateway, NULL, IoTP_Handler_Commands, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set global command handler. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}


/* Subscribes to command */
IOTPRC IoTPGateway_subscribeToCommands(IoTPGateway *gateway, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !gateway || !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Received NULL argument. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }

    /* Set topic string */
    int tlen = strlen(commandId) + strlen(formatString) + 16;
    char topic[tlen];
    snprintf(topic, tlen, "iot-2/cmd/%s/fmt/%s", commandId, formatString);

    LOG(DEBUG,"Subscribe command. topic: %s", topic);

    rc = iotp_client_subscribe((void *)gateway, topic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Sets a callback handler and subscribe to a command */
IOTPRC IoTPGateway_handleCommand(IoTPGateway *gateway, IoTPCallbackHandler cb, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !gateway || !cb || !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL argument.  rc: %d | reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }

    /* check for wild card character - not supported by this API */
    if ( strstr(commandId, "+")) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "Wild card is not supported. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }

    /* set command handler */
    rc = iotp_client_setHandler((void *)gateway, commandId, IoTP_Handler_Command, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set command handler. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
    }

    /* Subscribe to the command */
    int tlen = strlen(commandId) + strlen(formatString) + 16;
    char topic[tlen];
    snprintf(topic, tlen, "iot-2/cmd/%s/fmt/%s", commandId, formatString);

    LOG(DEBUG,"Subscribe command. topic: %s", topic);

    rc = iotp_client_subscribe((void *)gateway, topic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}


/* Unsubscribes from command */
IOTPRC IoTPGateway_unsubscribeFromCommands(IoTPGateway *gateway, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !gateway || !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL argument.  rc: %d | reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }

    int tlen = strlen(commandId) + strlen(formatString) + 16;
    char topic[tlen];
    snprintf(topic, tlen, "iot-2/cmd/%s/fmt/%s", commandId, formatString);

    LOG(DEBUG,"Unsubscribe command. topic: %s", topic);

    rc = iotp_client_unsubscribe((void *)gateway, topic);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to unsubscribe. topic: %s | rc: %d | reason: %s", topic, rc, IOTPRC_toString(rc));
    }

    return rc;
}


/* Subscribes to a command on behalf of a device */
IOTPRC IoTPGateway_subscribeToDeviceCommands(IoTPGateway *gateway, char *typeId, char *deviceId, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !gateway || !typeId || *typeId == '\0' || !deviceId || *deviceId == '\0' || !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL argument. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }

    int tlen = strlen(typeId) + strlen(deviceId) + strlen(commandId) + strlen(formatString) + 26;
    char topic[tlen];
    snprintf(topic, tlen, "iot-2/type/%s/id/%s/cmd/%s/fmt/%s", typeId, deviceId, commandId, formatString);

    LOG(DEBUG,"Subscribe command. topic: %s", topic);

    rc = iotp_client_subscribe((void *)gateway, topic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe. topic: %s | rc: %d | reason: %s", topic, rc, IOTPRC_toString(rc));
    }

    return rc;
}


/* Unsubscribes from device command */
IOTPRC IoTPGateway_unsubscribeFromDeviceCommands(IoTPGateway *gateway, char *typeId, char *deviceId, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !gateway || !typeId || *typeId == '\0' || !deviceId || *deviceId == '\0' || !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL argument. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }

    int tlen = strlen(typeId) + strlen(deviceId) + strlen(commandId) + strlen(formatString) + 26;
    char topic[tlen];
    snprintf(topic, tlen, "iot-2/type/%s/id/%s/cmd/%s/fmt/%s", typeId, deviceId, commandId, formatString);

    LOG(DEBUG,"Unsubscribe command. topic: %s", topic);

    rc = iotp_client_unsubscribe((void *)gateway, topic);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to unsubscribe. topic: %s | rc: %d | reason: %s", topic, rc, IOTPRC_toString(rc));
    }

    return rc;
}


/* Sets notification handler */
IOTPRC IoTPGateway_setNotificationHandler(IoTPGateway *gateway, IoTPCallbackHandler cb)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setHandler((void *)gateway, NULL, IoTP_Handler_Notification, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set handler. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}


/* Subscribes to notifications */
IOTPRC IoTPGateway_subscribeToNotifications(IoTPGateway *gateway, char *typeId, char *deviceId)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !gateway || !typeId || *typeId == '\0' || !deviceId || *deviceId == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Invalid or NULL arguments. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }

    /* Set topic string */
    int tlen = strlen(typeId) + strlen(deviceId) + 23;
    char topic[tlen];
    snprintf(topic, tlen, "iot-2/type/%s/id/%s/notify", typeId, deviceId);

    LOG(DEBUG,"Subscribe notification. topic: %s", topic);

    rc = iotp_client_subscribe((void *)gateway, topic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe. topic: %s | rc: %d | reason: %s", topic, rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Unsubscribes from notifications */
IOTPRC IoTPGateway_unsubscribeFromNotifications(IoTPGateway *gateway, char *typeId, char *deviceId)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !gateway || !typeId || *typeId == '\0' || !deviceId || *deviceId == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Invalid or NULL arguments. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }

    /* Set topic string */
    int tlen = strlen(typeId) + strlen(deviceId) + 23;
    char topic[tlen];
    snprintf(topic, tlen, "iot-2/type/%s/id/%s/notify", typeId, deviceId);

    LOG(DEBUG,"Unsubscribe notification. topic: %s", topic);

    rc = iotp_client_unsubscribe((void *)gateway, topic);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to unsubscribe. topic: %s | rc: %d | reason: %s", topic, rc, IOTPRC_toString(rc));
    }

    return rc;
}


/* Sets handler for monitoring message */
IOTPRC IoTPGateway_setMonitoringMessageHandler(IoTPGateway *gateway, IoTPCallbackHandler cb)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setHandler((void *)gateway, NULL, IoTP_Handler_MonitoringMessage, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set handler. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}


/* Subcsribes to monitoring messages */
IOTPRC IoTPGateway_subscribeToMonitoringMessages(IoTPGateway *gateway, char *typeId, char *deviceId)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !gateway || !typeId || *typeId == '\0' || !deviceId || *deviceId == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Invalid or NULL arguments. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }

    /* Set topic string */
    int tlen = strlen(typeId) + strlen(deviceId) + 20;
    char topic[tlen];
    snprintf(topic, tlen, "iot-2/type/%s/id/%s/mon", typeId, deviceId);

    LOG(DEBUG,"Subscribe monitoring message. topic: %s", topic);

    rc = iotp_client_subscribe((void *)gateway, topic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe. topic: %s | rc: %d | reason: %s", topic, rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Unsubscribes from monitoring messages */
IOTPRC IoTPGateway_unsubscribeFromMonitoringMessages(IoTPGateway *gateway, char *typeId, char *deviceId)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !gateway || !typeId || *typeId == '\0' || !deviceId || *deviceId == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Invalid or NULL arguments. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }

    /* Set topic string */
    int tlen = strlen(typeId) + strlen(deviceId) + 20;
    char topic[tlen];
    snprintf(topic, tlen, "iot-2/type/%s/id/%s/mon", typeId, deviceId);

    LOG(DEBUG,"Unsubscribe monitoring message. topic: %s", topic);

    rc = iotp_client_unsubscribe((void *)gateway, topic);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to unsubscribe. topic: %s | rc: %d | reason: %s", topic, rc, IOTPRC_toString(rc));
    }

    return rc;
}

