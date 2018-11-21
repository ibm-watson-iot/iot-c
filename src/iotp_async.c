/*******************************************************************************
 * Copyright (c) 2018 IBM Corp.
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

#include <MQTTAsync.h>

#include "iotp_utils.h"
#include "iotp_internal.h"

static int iotp_client_messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message * message);

/* Add callback handler to the handler list */
static IoTP_RC iotp_add_handler(IoTPHandlers * handlers, IoTPHandler * handler) 
{
    IoTP_RC rc = IoTP_SUCCESS;
    int i = 0;

    /* Initialize handlers table */
    if (handlers->count == handlers->nalloc) {
        IoTPHandler ** tmp = NULL;
        int firstSlot = handlers->nalloc;
        handlers->nalloc = handlers->nalloc == 0 ? 8 : handlers->nalloc * 2;
        tmp = realloc(handlers->entries, sizeof(IoTPHandler *) * handlers->nalloc);
        if (tmp == NULL) {
            return IoTP_RC_NOMEM;
        }
        handlers->entries = tmp;
        for (i = firstSlot; i < handlers->nalloc; i++)
            handlers->entries[i] = NULL;
        handlers->slots = handlers->count;
    }

    /* Add handler entry */
    if (handlers->count == handlers->slots) {
        handlers->entries[handlers->count] = handler;
        handlers->id = handlers->count;
        handlers->count++;
        handlers->slots++;
    } else {
        for (i = 0; i < handlers->slots; i++) {
            if (!handlers->entries[i]) {
                handlers->entries[i] = handler;
                handlers->id = i;
                handlers->count++;
                break;
            }
        }
    }

    return rc;
}

/* Find callback handler */
static IoTPHandler * iotp_client_getHandler(IoTPHandlers * handlers, char * commandId) {
    int i = 0;
    for (i = 0; i < handlers->count; i++) {
        IoTPHandler * handler = handlers->entries[i];
        if (commandId && handler->topic && strcmp(commandId, handler->topic) == 0) {
            return handler;
        }
    }
    return NULL;
}

/* Return handler type string */
static char * iotp_client_getHanglerTypeStr(IoTP_Handler_type_t type)
{
    switch(type) {
        case IoTP_Handler_GlobalCommand: return "GlobalCommand";
        case IoTP_Handler_Command: return "Command";
        case IoTP_Handler_Notification: return "Notification";
        case IoTP_Handler_MonitoringMessage: return "MonitoringMessage";
        case IoTP_Handler_DeviceMonitoring: return "DeviceMonitoring";
        case IoTP_Handler_AppMonitoring: return "AppMonitoring";
        case IoTP_Handler_Event: return "Event";
    }

    return "Unknown";
}


/* Callback function to handle connection lose cases */
void connlost(void *context, char *cause)
{
    LOG(TRACE, "entry::");
    LOG(WARN, "IoTP client connection is lost. Context=%x Cause=%s", context, cause);

    int auto_reconnect = 1;
    if ( auto_reconnect ) {
        LOG(INFO, "Auto reconnect is enabled. Reconnecting ...");

        MQTTAsync client = (MQTTAsync)context;
        MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
        int rc;

        conn_opts.keepAliveInterval = 60;
        conn_opts.cleansession = 1;

        if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
        {
            LOG(ERROR, "Failed to connect, rc=%d", rc);
        }
    }

    LOG(TRACE, "exit::");
}

/* Callback function to process successful disconnection */
void onDisconnect(void *context, MQTTAsync_successData *response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = client->clientId;
    LOG(INFO, "Client (id=%s) is successfully disconnected.", clientId? clientId:"NULL");
    client->connected = 0;
}

/* Callback function to process successful connection */
void onConnect(void *context, MQTTAsync_successData *response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = client->clientId;
    LOG(INFO, "Client (id=%s) is connected successfully.", clientId? clientId:"NULL");
    client->connected = 1;
}

/* Callback function to process connection failure */
void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = client->clientId;
    LOG(INFO, "Client (id=%s) connection has failed. rc=%d", clientId? clientId:"NULL", response? response->code:0);
    client->connected = 0;
}

/* Callback function to process successful send */
void onSend(void *context, MQTTAsync_successData *response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = client->clientId;
    LOG(TRACE, "Event is sent from client (id=%s).", clientId? clientId:"NULL");
}

/* Callback function to process send failure */
void onSendFailure(void *context, MQTTAsync_failureData *response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = client->clientId;
    LOG(WARN, "Failed to send event from client (id=%s). rc=%d", clientId? clientId:"NULL", response? response->code:0);
}

/* Callback function to process successful subscription */
void onSubscribe(void* context, MQTTAsync_successData* response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = client->clientId;
    LOG(INFO, "Client (id=%s) has subscribe to the topic: QoS=%d", clientId? clientId:"NULL", response->alt.qos);
}

/* Callback function to process subscription failure */
void onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = client->clientId;
    LOG(WARN, "Client (id=%s) has failed to subscribe to the topic: rc=%d", clientId? clientId:"NULL", response? response->code:0);
}

/* Callback function to process successful unsubscribed a topic */
void onUnSubscribe(void* context, MQTTAsync_successData* response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = client->clientId;
    LOG(INFO, "Client (id=%s) has unsubscribe from the topic", clientId? clientId:"NULL");
}

/* Callback function to process failure from unsubscribe call */
void onUnSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = client->clientId;
    LOG(WARN, "Client (id=%s) has failed to unsubscribe from the topic: rc=%d", clientId? clientId:"NULL", response? response->code:0);
}

/* Creates IoTPClient */
/* TODO: check for errors and return error codes, add logs */
IoTP_RC iotp_client_create(void **iotpClient, IoTPConfig *config, IoTPClientType type)
{
    IoTP_RC rc = IoTP_SUCCESS;

    /* set connection URI */
    char *connectionURI = NULL;
    char *clientId = NULL;
    int len = 0;

    /* Check if client and config handles are valid */
    if ( *iotpClient != NULL || config == NULL ) {
        rc = IoTP_RC_INVALID_HANDLE;
        LOG(ERROR, "Invalid (%s) handle (%s) or config handle (%s)", IoTPClient_names[type], *iotpClient?"Invalid":"Valid", config?"Valid":"Invalid");
        return rc;
    }
    /* Validate client type */
    if ( type < 1 && type > IoTPClient_total )  {
        rc = IoTP_RC_PARAM_INVALID_VALUE;
        LOG(ERROR, "Invalid IoTP client type %d", type);
        return rc;
    }

    char *domain = config->domain;
    int port = config->port;
    char *orgId = config->orgId;
    int orgIdLen = strlen(orgId);

    len = orgIdLen + strlen(domain) + 23;   /* 23 = 6 for ssl:// + 11 for .messaging. + 5 for :port + 1 */
    connectionURI = (char *)malloc(len);
    if ( port == 1883 ) {
        snprintf(connectionURI, len, "tcp://%s.messaging.%s:%d", orgId, domain, port);
    } else {
        snprintf(connectionURI, len, "ssl://%s.messaging.%s:%d", orgId, domain, port);
    }

    LOG(INFO, "OrganizationID: %s", orgId);
    LOG(INFO, "ConnectionURI: %s", connectionURI);

    /* set client id */
    if ( type == IoTPClient_device || type == IoTPClient_managed_device ) {
        len = orgIdLen + strlen(config->device.typeId) + strlen(config->device.deviceId) + 5;
        clientId = malloc(len);
        snprintf(clientId, len, "d:%s:%s:%s", orgId, config->device.typeId, config->device.deviceId);
    } else if ( type == IoTPClient_gateway || type == IoTPClient_managed_gateway ) {
        len = orgIdLen + strlen(config->gateway.typeId) + strlen(config->gateway.deviceId) + 5;
        clientId = malloc(len);
        snprintf(clientId, len, "g:%s:%s:%s", orgId, config->gateway.typeId, config->gateway.deviceId);
    } else if ( type == IoTPClient_application ) {
        len = orgIdLen + strlen(config->application.appId) + 4;
        clientId = malloc(len);
        snprintf(clientId, len, "a:%s:%s", orgId, config->application.appId);
    } else if ( type == IoTPClient_Application ) {
        len = orgIdLen + strlen(config->application.appId) + 4;
        clientId = malloc(len);
        snprintf(clientId, len, "A:%s:%s", orgId, config->application.appId);
    }

    LOG(INFO, "ClientId: %s", clientId);

    IoTPClient *client = (IoTPClient *)calloc(1, sizeof(IoTPClient));
    client->type = type;
    client->config = config;
    client->clientId = clientId;
    client->connectionURI = connectionURI;
    client->mqttClient = NULL;
    client->handlers = (IoTPHandlers *) calloc(1, sizeof(IoTPHandlers));
    client->inited = 1;
    client->keepAliveInterval = 60;

    *iotpClient = client;

    return rc;
}

/* Sets log handler for IoTP and MQTTAsync client */
IoTP_RC iotp_client_setMQTTLogHandler(void *iotpClient, IoTPLogHandler *cb) 
{
    IoTP_RC rc = IoTP_SUCCESS;

    /* Check if client handle is valid */
    if ( iotpClient == NULL ) {
        rc = IoTP_RC_INVALID_HANDLE;
        return rc;
    } 

    IoTPClient *client = (IoTPClient *)iotpClient;

    /* set MQTTAsync trace callback */
    if ( client->config->MQTTTraceLevel > 0 ) {
        MQTTAsync_setTraceLevel(client->config->MQTTTraceLevel);
        MQTTAsync_setTraceCallback((MQTTAsync_traceCallback *)cb);
    }

    return rc;
}
 


IoTP_RC iotp_client_destroy(void *iotpClient)
{
    IoTP_RC rc = IoTP_SUCCESS;

    /* Check if client handle is valid */
    if ( iotpClient == NULL ) {
        rc = IoTP_RC_INVALID_HANDLE;
        return rc;
    } 

    IoTPClient *client = (IoTPClient *)iotpClient;
    IoTPHandlers *handlers = NULL;
    int i = 0;

    /* disconnect client - ignore errors */
    if ( iotp_client_disconnect(iotpClient) != IoTP_SUCCESS ) {
        LOG(WARN, "Failed to disconnect the client.");
    }

    iotp_utils_freePtr((void *)client->clientId);
    iotp_utils_freePtr((void *)client->connectionURI);
    handlers = client->handlers;

    for (i=0; i<handlers->count; i++)
    {
        IoTPHandler * sub = handlers->entries[i];
        iotp_utils_freePtr((void *)sub->topic);
    }

    iotp_utils_freePtr((void *)handlers);
    iotp_utils_freePtr((void *)client);
    client = NULL;

    return rc;
}

/* Connect MQTT Async client */
/* TODO: check for errors and return error codes, add logs */
IoTP_RC iotp_client_connect(void *iotpClient)
{
    IoTP_RC rc = IoTP_SUCCESS;
    IoTPClient *client = (IoTPClient *)iotpClient;

    /* Sanity check */
    if ( !client ) {
        rc = IoTP_RC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle. rc=%d", rc);
        return rc;
    }

    MQTTAsync                   mqttClient;
    MQTTAsync_connectOptions    conn_opts = MQTTAsync_connectOptions_initializer;
    MQTTAsync_SSLOptions        ssl_opts = MQTTAsync_SSLOptions_initializer;
    /* MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer; */
    MQTTAsync_createOptions     create_opts = MQTTAsync_createOptions_initializer;
    /* MQTTAsync_willOptions       will_opts = MQTTAsync_willOptions_initializer; */

    int port = client->config->port;

    /* Create MQTT Async client - by default use MQTT V5 */
    create_opts.MQTTVersion = MQTTVERSION_5;
    create_opts.sendWhileDisconnected = 1;

    LOG(INFO, "ConnectionURI: %s", client->connectionURI);
    LOG(INFO, "ClientID: %s", client->clientId);

    rc = MQTTAsync_createWithOptions(&mqttClient, client->connectionURI, client->clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL, &create_opts);
    if ( rc != MQTTASYNC_SUCCESS ) {
        LOG(WARN, "MQTTAsync_createWithOptions returned rc=%d",rc);
        return rc;
    }

    client->mqttClient = (void *)mqttClient;

    /* set connection options */
    conn_opts.keepAliveInterval = client->keepAliveInterval;
    conn_opts.cleansession = 1;
    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.MQTTVersion = 4;
    conn_opts.context = client;
    conn_opts.automaticReconnect = 1;

    /* set authentication credentials */
    ssl_opts.enableServerCertAuth = 0;

    if ( port == 8883 || port == 443 ) {
        /* set ssl */
        conn_opts.ssl = &ssl_opts;

        if ( client->type == IoTPClient_application || client->type == IoTPClient_Application ) {
            conn_opts.username = client->config->application.APIKey;
            conn_opts.password = client->config->application.authToken;
        } else if ( client->type == IoTPClient_device || client->type == IoTPClient_managed_device ) {
            if ( client->config->device.authToken ) {
                conn_opts.username = "use-token-auth";
                conn_opts.password = client->config->device.authToken;
            }
            if ( client->config->device.certificatePath ) {
                conn_opts.ssl->enableServerCertAuth = 1;
                conn_opts.ssl->trustStore = client->config->serverCertificatePath;
                conn_opts.ssl->keyStore = client->config->device.certificatePath;
                conn_opts.ssl->privateKey = client->config->device.keyPath;
            }
        } else if ( client->type == IoTPClient_gateway || client->type == IoTPClient_managed_gateway ) {
            if ( client->config->gateway.authToken ) {
                conn_opts.username = "use-token-auth";
                conn_opts.password = client->config->gateway.authToken;
            }
            if ( client->config->gateway.certificatePath ) {
                conn_opts.ssl->enableServerCertAuth = 1;
                conn_opts.ssl->trustStore = client->config->serverCertificatePath;
                conn_opts.ssl->keyStore = client->config->gateway.certificatePath;
                conn_opts.ssl->privateKey = client->config->gateway.keyPath;
            }
        }
    }
    
    
    /* Set callbacks */
    MQTTAsync_setCallbacks((MQTTAsync *)client->mqttClient, (void *)client, NULL, iotp_client_messageArrived, NULL);
           
    if ((rc = MQTTAsync_connect((MQTTAsync *)client->mqttClient, &conn_opts)) == MQTTASYNC_SUCCESS) {
        LOG(INFO, "MQTTAsync_connect is called: ClientType=%d ClientId=%s  ConnectionURI=%s", 
            client->type, client->clientId, client->connectionURI);
    } else {
        LOG(INFO, "MQTTAsync_connect returned error: ClientType=%d ClientId=%s  ConnectionURI=%s rc=%d", 
            client->type, client->clientId, client->connectionURI, rc);
    }

    return rc;
}


/* Function used to publish the given data to the topic with the given QoS */
IoTP_RC iotp_client_publish(void *iotpClient, char *topic, char *payload, int qos)
{
    IoTP_RC rc = IoTP_SUCCESS;
    IoTPClient *client = (IoTPClient *)iotpClient;

    /* Sanity check */
    if ( !client || !client->config ) {
        rc = IoTP_RC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle. rc=%d", rc);
        return rc;
    }

    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    MQTTAsync mqttClient = (MQTTAsync *)client->mqttClient;

    opts.onSuccess = onSend;
    opts.onFailure = onSendFailure;
    opts.context = client;

    int payloadlen = strlen(payload);

    pubmsg.payload = payload;
    pubmsg.payloadlen = payloadlen;
    pubmsg.qos = qos;
    pubmsg.retained = 0;

    LOG(DEBUG, "Publish Message: qos=%d retained=%d payloadlen=%d payload: %s",
                    pubmsg.qos, pubmsg.retained, pubmsg.payloadlen, payload);

    rc = MQTTAsync_send(mqttClient, topic, payloadlen, payload, qos, 0, &opts);
    if ( rc != MQTTASYNC_SUCCESS ) {
        LOG(ERROR, "MQTTAsync_send returned error: rc=%d", rc);
        LOG(WARN, "Connection lost, retry connection and republish message.\n");
        iotp_client_retry_connection(mqttClient);
        rc = MQTTAsync_send(mqttClient, topic, payloadlen, payload, qos, 0, &opts);
    }

    return rc;
}

/* Function to subscribe to an MQTT topic to get command from WIoTP. */
IoTP_RC iotp_client_subscribe(void *iotpClient, char *topic, int qos)
{
    IoTP_RC rc = IoTP_SUCCESS;
    IoTPClient *client = (IoTPClient *)iotpClient;

    /* Sanity check */
    if ( !client || !client->config ) {
        rc = IoTP_RC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle. rc=%d", rc);
        return rc;
    }

    /* check if client is connected */
    int l = 0;
    /* try 5 times */
    for (l=0; l<5; l++) {
        if (client->connected == 0 ) {
            LOG(WARN, "Client is not connected yet. Wait for client to connect and subscribe.");
            iotp_utils_delay(2000);
        }
    }

    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync mqttClient = (MQTTAsync *)client->mqttClient;

    opts.onSuccess = onSubscribe;
    opts.onFailure = onSubscribeFailure;
    opts.context = client;

    LOG(DEBUG,"Subscribe to topic=%s qos=%d", topic, qos);

    rc = MQTTAsync_subscribe(mqttClient, topic, qos, &opts);

    if ( rc != MQTTASYNC_SUCCESS ) {
        LOG(ERROR, "MQTTAsync_subscribe returned error: rc=%d", rc);
    }

    return rc;
}

/* Function to unsubscribe to an MQTT topic to get command from WIoTP. */
IoTP_RC iotp_client_unsubscribe(void *iotpClient, char *topic)
{
    IoTP_RC rc = IoTP_SUCCESS;
    IoTPClient *client = (IoTPClient *)iotpClient;

    MQTTAsync asyncClient = (MQTTAsync *)client->mqttClient;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;

    opts.onSuccess = onUnSubscribe;
    opts.onFailure = onUnSubscribeFailure;
    opts.context = asyncClient;

    LOG(DEBUG,"UnSubscribe topic=%s", topic);

    rc = MQTTAsync_unsubscribe((MQTTAsync *)client->mqttClient, topic, &opts);
    if ( rc != MQTTASYNC_SUCCESS ) {
        LOG(ERROR, "MQTTAsync_unsubscribe returned error: rc=%d", rc);
    }

    return rc;
}

/* Function used to check if the client is connected */
IoTP_RC iotp_client_isConnected(void *iotpClient)
{
    IoTP_RC rc = IoTP_SUCCESS;
    IoTPClient *client = (IoTPClient *)iotpClient;

    /* Sanity check */
    if ( !client || !client->config ) {
        rc = IoTP_RC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle. rc=%d", rc);
        return rc;
    }

    rc = MQTTAsync_isConnected(client->mqttClient);

    return rc;
}

/* Set the Handler (callback function). This must be set if you want to recieve commands */
IoTP_RC iotp_client_setHandler(void *iotpClient, char *topic, int type, IoTPCallbackHandler handler)
{
    IoTP_RC rc = 0;
    IoTPClient *client = (IoTPClient *)iotpClient;

    /* Sanity check */
    if ( !client || !client->config ) {
        rc = IoTP_RC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle. rc=%d", rc);
        return rc;
    }

    /* TODO: error conditions to handle:
     * If type=IoTP_Handler_CommandGlobal and topic is not NULL
     */

    /* Add handler to the list. */
    if ( client->handlers->count == 0 ) {
        IoTPHandler * handler = (IoTPHandler *)calloc(1, sizeof(IoTPHandler));
        handler->type = type;
        handler->topic = NULL;
        if ( topic && *topic != '\0' ) {
            handler->topic = strdup(topic);
        }
        handler->cbFunc = handler;
        rc = iotp_add_handler(client->handlers, handler);
        if ( rc == IoTP_SUCCESS ) {
            LOG(INFO, "Handler (type=%s) is added. Topic=%s", iotp_client_getHanglerTypeStr(type), topic? topic:"NULL");
        } else {
            LOG(INFO, "Failed to add handler (type=%s) for topic=%s rc=%d", iotp_client_getHanglerTypeStr(type), topic? topic:"NULL", rc);
        }

        return rc;
        
    }

    /* Check if command handler is already set for commandId */
    /* Loop thru the list */
     if ( client->handlers->count == 1 ) {
        IoTPHandler * handler = client->handlers->entries[0];
        if ( handler->type == IoTP_Handler_GlobalCommand ) {
            handler->cbFunc = handler;
            LOG(INFO, "Global callback is updated.");
        }

    } else {
        /* Global can not be set - individual handlers exist */
        rc = IoTP_FAILURE;
        LOG(INFO, "Can not register global callback. rc=%d", rc);
    }

    return rc;
}

/* Handle received messages - invoke the callback. */
static int iotp_client_messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message * message)
{
    IoTP_RC rc = IoTP_SUCCESS;
    IoTPCallbackHandler cb = NULL;
    IoTPClient *client = (IoTPClient *)context;

    /* Sanity check */
    if ( !client || !client->config ) {
        rc = IoTP_RC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle. rc=%d", rc);
        return 0;
    }

    /* Get callback */
    if ( client->handlers->count == 0 ) {
        /* no callback is configured */
        LOG(TRACE, "Message Received on topic: %s", topicName? topicName:"");        
        LOG(TRACE, "Message handler is not confogured. Client can not process received messages.");
        return 0;
    }

    IoTPHandler * sub = client->handlers->entries[0];
    /* check for global callback */
    if ( sub->type == IoTP_Handler_GlobalCommand ) { 
        cb = sub->cbFunc;
    } else {
        /* search callback based on topicName */
        sub = iotp_client_getHandler(client->handlers, topicName);
        if ( sub == NULL ) {
            /* no callback is configured */
            LOG(TRACE, "Message Received on topic: %s", topicName? topicName:"");        
            LOG(TRACE, "Message handler is not configured. Client can not process received messages.");
            return 0;
        }

        cb = sub->cbFunc;
    }

    /* Process incoming message if callback is defined */
    if (cb != 0) {
        char topic[4096];
        void *payload = message->payload;
        size_t payloadlen = message->payloadlen;

        snprintf(topic,4096, "%s", topicName);

        LOG(INFO, "Context:%x Topic:%s TopicLen=%d PayloadLen=%d Payload:%s", context, topic, topicLen, payloadlen, payload);

        char *type = NULL;
        char *id = NULL;
        char *commandName = NULL;
        char *format = NULL;

        if ( strncmp(topicName, "iot-2/cmd/", 10) == 0 ) {

            strtok(topic, "/");
            strtok(NULL, "/");
            commandName = strtok(NULL, "/");
            strtok(NULL, "/");
            format = strtok(NULL, "/");

        } else {

            strtok(topic, "/");
            strtok(NULL, "/");

            type = strtok(NULL, "/");
            strtok(NULL, "/");
            id = strtok(NULL, "/");
            strtok(NULL, "/");
            commandName = strtok(NULL, "/");
            strtok(NULL, "/");
            format = strtok(NULL, "/");

        }

        LOG(TRACE, "Calling registered callabck to process the arrived message");

        (*cb)(type,id,commandName, format, payload,payloadlen);

        MQTTAsync_freeMessage(&message);
        MQTTAsync_free(topicName);
    } else {
        LOG(TRACE, "No registered callback function to process the arrived message");
    }

    return 1;
}


/* Disconnect from the IBM Watson IoT service */
IoTP_RC iotp_client_disconnect(void *iotpClient)
{
    IoTP_RC rc = 0;
    IoTPClient *client = (IoTPClient *)iotpClient;

    /* Sanity check */
    if ( !client ) {
        rc = IoTP_RC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle. rc=%d", rc);
        return rc;
    }

    MQTTAsync mqttClient = (MQTTAsync *)client->mqttClient;
    MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;

    opts.onSuccess = onDisconnect;
    opts.context = client;

    if (iotp_client_isConnected(iotpClient)) {
        rc = MQTTAsync_disconnect(mqttClient, &opts);
    }

    return rc;
}


/* increase delay amount based on tries */
static int reconnect_delay(int i)
{
    if (i < 10) {
        return 3; /* first 10 attempts try every 3 seconds */
    }
    if (i < 20) {
        return 60; /* next 10 attempts retry after every 1 minute */
    }

    return 600; /* after 20 attempts, retry every 10 minutes */
}

/* Retry connection */
IoTP_RC iotp_client_retry_connection(void *iotpClient)
{
    IoTP_RC rc = IoTP_SUCCESS;
    int retry = 1;

    while((rc = iotp_client_connect(iotpClient)) != MQTTASYNC_SUCCESS)
    {
        LOG(DEBUG, "Retry Attempt #%d ", retry);
        int delay = reconnect_delay(retry++);
        LOG(DEBUG, " next attempt in %d seconds\n", delay);
        sleep(delay);
    }

    return rc;
}


