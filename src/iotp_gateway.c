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

/* Gateway APIs are essentially wrapper functions of internal iotp_client_*() functions */

/* Creates a gateway handle */
IOTPRC IoTPGateway_create(IoTPGateway **gateway, IoTPConfig *config) 
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_create((void **)gateway, config, IoTPClient_gateway);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to create IoTPGateway: rc=%d", rc);
    }

    return rc;
}

/* Set MQTT Trace handler */
IOTPRC IoTPGateway_setMQTTLogHandler(IoTPGateway *gateway, IoTPLogHandler *cb)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setMQTTLogHandler((void *)gateway, cb);
    return rc;
}

/* Disconnect and destroys a gateway handle */
IOTPRC IoTPGateway_destroy(IoTPGateway *gateway)
{
    IOTPRC rc = IOTPRC_SUCCESS;
    int destroyMQTTClient = 1;

    rc = iotp_client_destroy((void *)gateway, destroyMQTTClient);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to destory IoTPGateway: rc=%d", rc);
    }

    return rc;
}

/* Connects to WIoTP */
IOTPRC IoTPGateway_connect(IoTPGateway *gateway)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_connect((void *)gateway);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Gateway failed to connect");
    }

    return rc;
}


/* Disconnect client from WIoTP */
IOTPRC IoTPGateway_disconnect(IoTPGateway *gateway)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_disconnect((void *)gateway);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Gateway failed to disconnect");
    }

    return rc;
}


IOTPRC IoTPGateway_sendEvent(IoTPGateway *gateway, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !eventId || *eventId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL arguments, rc=%d", rc);
        return rc;
    }

    /* get device type and id of this gateway object */
    char *typeId     = iotp_client_getDeviceType((void *)gateway);
    char *deviceId   = iotp_client_getDeviceId((void *)gateway);

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

    rc = iotp_client_publish((void *)gateway, publishTopic, data, qos, props);

    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to send event: %s rc=%d", eventId, rc);
    }

    return rc;
}



IOTPRC IoTPGateway_sendDeviceEvent(IoTPGateway *gateway, char *typeId, char *deviceId, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props)
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

    rc = iotp_client_publish((void *)gateway, publishTopic, data, qos, props);

    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to send event: %s rc=%d", eventId, rc);
    }

    return rc;
}


IOTPRC IoTPGateway_setCommandHandler(IoTPGateway *gateway, IoTPCallbackHandler cb)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setHandler((void *)gateway, NULL, IoTP_Handler_Commands, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set global command handler");
    }

    return rc;
}


IOTPRC IoTPGateway_subscribeToCommands(IoTPGateway *gateway, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL arguments, rc=%d", rc);
        return rc;
    }

    int tlen = strlen(commandId) + strlen(formatString) + 16;
    char subTopic[tlen];
    snprintf(subTopic, tlen, "iot-2/cmd/%s/fmt/%s", commandId, formatString);

    rc = iotp_client_subscribe((void *)gateway, subTopic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe to the command: %s rc=%d", commandId, rc);
    }

    return rc;
}


IOTPRC IoTPGateway_handleCommand(IoTPGateway *gateway, IoTPCallbackHandler cb, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !cb || !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL arguments, rc=%d", rc);
        return rc;
    }

    /* check for wild card character - not supported by this API */
    if ( strstr(commandId, "+")) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "Wild card character is not supported. rc=%d", rc);
        return rc;
    }

    /* set command handler */
    rc = iotp_client_setHandler((void *)gateway, commandId, IoTP_Handler_Command, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set command handler");
    }

    /* Subscribe to the command */
    int tlen = strlen(commandId) + strlen(formatString) + 16;
    char subTopic[tlen];
    snprintf(subTopic, tlen, "iot-2/cmd/%s/fmt/%s", commandId, formatString);

    rc = iotp_client_subscribe((void *)gateway, subTopic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe to the command: %s rc=%d", subTopic, rc);
    }

    return rc;
}


IOTPRC IoTPGateway_unsubscribeFromCommands(IoTPGateway *gateway, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL arguments, rc=%d", rc);
        return rc;
    }

    int tlen = strlen(commandId) + strlen(formatString) + 16;
    char subTopic[tlen];
    snprintf(subTopic, tlen, "iot-2/cmd/%s/fmt/%s", commandId, formatString);

    rc = iotp_client_unsubscribe((void *)gateway, subTopic);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to unsubscribe to the command: %s rc=%d", subTopic, rc);
    }

    return rc;
}


IOTPRC IoTPGateway_subscribeToDeviceCommands(IoTPGateway *gateway, char *typeId, char *deviceId, char *commandId, char *formatString)
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

    rc = iotp_client_subscribe((void *)gateway, subTopic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe to the command: %s rc=%d", subTopic, rc);
    }

    return rc;
}


IOTPRC IoTPGateway_unsubscribeFromDeviceCommands(IoTPGateway *gateway, char *typeId, char *deviceId, char *commandId, char *formatString)
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

    rc = iotp_client_unsubscribe((void *)gateway, subTopic);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe to the command: %s rc=%d", subTopic, rc);
    }

    return rc;
}


IOTPRC IoTPGateway_setNotificationHandler(IoTPGateway *gateway, IoTPCallbackHandler cb)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setHandler((void *)gateway, NULL, IoTP_Handler_Notification, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set notification handler");
    }

    return rc;
}


IOTPRC IoTPGateway_subscribeToNotifications(IoTPGateway *gateway, char *typeId, char *deviceId)
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

    rc = iotp_client_subscribe((void *)gateway, subTopic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe to the notification: %s rc=%d", subTopic, rc);
    }

    return rc;
}



IOTPRC IoTPGateway_unsubscribeFromNotifications(IoTPGateway *gateway, char *typeId, char *deviceId)
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

    rc = iotp_client_unsubscribe((void *)gateway, subTopic);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe to the notification: %s rc=%d", subTopic, rc);
    }

    return rc;
}



IOTPRC IoTPGateway_setMonitoringMessageHandler(IoTPGateway *gateway, IoTPCallbackHandler cb)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setHandler((void *)gateway, NULL, IoTP_Handler_MonitoringMessage, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set monitoring message handler");
    }

    return rc;
}


IOTPRC IoTPGateway_subscribeToMonitoringMessages(IoTPGateway *gateway, char *typeId, char *deviceId)
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

    rc = iotp_client_subscribe((void *)gateway, subTopic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe to the monitoring messages: %s rc=%d", subTopic, rc);
    }

    return rc;
}

IOTPRC IoTPGateway_unsubscribeFromMonitoringMessages(IoTPGateway *gateway, char *typeId, char *deviceId)
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

    rc = iotp_client_unsubscribe((void *)gateway, subTopic);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe to the monitoring messages: %s rc=%d", subTopic, rc);
    }

    return rc;
}

