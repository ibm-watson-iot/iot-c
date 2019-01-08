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

/* Validate client configuration */
static IoTP_RC iotp_validate_config(int type, IoTPConfig *config)
{
    IoTP_RC rc = IoTP_SUCCESS;

    /* config, domain, or orgID */
    if ( !config ) {
        rc = IoTP_RC_PARAM_NULL_VALUE;
        return rc;
    }

    /* domain and org ID can not be NULL */
    if ( config->domain == NULL || *config->domain == '\0' ) {
        rc = IoTP_RC_PARAM_INVALID_VALUE;
        LOG(ERROR, "Configuration item domain is NULL or empty: rc=%d", rc);
        return rc;
    } else if ( config->identity->orgId == NULL || *config->identity->orgId == '\0' ) {
        if ( config->auth->apiKey == NULL || *config->auth->apiKey == '\0' ) {
            rc = IoTP_RC_PARAM_INVALID_VALUE;
            LOG(ERROR, "Configuration item orgId or API Key is NULL or empty: rc=%d", rc);
            return rc;
        } else {
            /* get orgid from API Key and update config */
            char *tmpstr = config->auth->apiKey;
            char *tok = strtok(tmpstr, "-");
            if ( tok != NULL ) {
                char *orgId = strtok(NULL, "-");
                config->identity->orgId = strdup(orgId);
            } else {
                LOG(ERROR, "Configuration item orgId or APIKey is NULL or empty or invalid: rc=%d", rc);
            }
        }
    }

    /* Check serverCertificatePath */
    /* Could be NULL for quickstart sandbox */
    if ( strcmp(config->domain, "quickstart") != 0 ) {
        if ( config->mqttopts->caFile == NULL || *config->mqttopts->caFile == '\0' ) {
            rc = IoTP_RC_PARAM_INVALID_VALUE;
            LOG(ERROR, "Configuration item certificatePath is NULL or empty: rc=%d", rc);
            return rc;
        }
        /* check if caFile is valid */
        if ( (rc = iotp_utils_fileExist(config->mqttopts->caFile)) != IoTP_SUCCESS ) {
            LOG(ERROR, "Invalid caFile (%s) is specified: rc=%d", config->mqttopts->caFile, rc);
            return rc;
        }
        /* check port - should be 8883 or 443 */
        if ( config->mqttopts->port != 8883 && config->mqttopts->port != 443 ) {
            rc = IoTP_RC_PARAM_INVALID_VALUE;
            LOG(ERROR, "Configuration item port (%d) is not valid: rc=%d", config->mqttopts->port, rc);
            return rc;
        }
    } else {
        /* check port for quickstart - should be 1883 */
        if ( config->mqttopts->port != 1883 ) {
            rc = IoTP_RC_PARAM_INVALID_VALUE;
            LOG(ERROR, "Configuration item port (%d) for quickstart domain is not valid: rc=%d", config->mqttopts->port, rc);
            return rc;
        }
    }

    /* Validate device or gateway related config items */
    if ( type == IoTPClient_device ) {
        /* device id and type can not be empty */
        if (config->identity->typeId == NULL || ( config->identity->typeId && *config->identity->typeId == '\0')) {
            rc = IoTP_RC_PARAM_NULL_VALUE;
            LOG(ERROR, "Device configuration typeId is NULL or empty: rc=%d", rc);
            return rc;
        }
        if (config->identity->deviceId == NULL || ( config->identity->deviceId && *config->identity->deviceId == '\0')) {
            rc = IoTP_RC_PARAM_NULL_VALUE;
            LOG(ERROR, "Device configuration deviceId is NULL or empty: rc=%d", rc);
            return rc;
        }
        /* validate device authMethod related config items */
        if ( config->authMethod == 0 ) {
            /* Should have valid authToken */
            if (config->auth->token == NULL || (config->auth->token && *config->auth->token == '\0')) {
                rc = IoTP_RC_PARAM_NULL_VALUE;
                LOG(ERROR, "Device configuration authToken is NULL or empty: rc=%d", rc);
                return rc;
            }
        } else {
            /* should have valid client cert and key */
            if (config->auth->keyStore == NULL || ( config->auth->keyStore && *config->auth->keyStore == '\0')) {
                rc = IoTP_RC_PARAM_NULL_VALUE;
                LOG(ERROR, "Device configuration keyStore is NULL or empty: rc=%d", rc);
                return rc;
            } else {
                /* check if file exist */
                if ( (rc = iotp_utils_fileExist(config->auth->keyStore)) != IoTP_SUCCESS ) {
                    LOG(ERROR, "Invalid device keyStore (%s) is specified: rc=%d", config->auth->keyStore, rc);
                    return rc;
                }
            }
            if (config->auth->privateKey == NULL || ( config->auth->privateKey && *config->auth->privateKey == '\0')) {
                rc = IoTP_RC_PARAM_NULL_VALUE;
                LOG(ERROR, "Device configuration privateKey is NULL or empty: rc=%d", rc);
                return rc;
            } else {
                /* check if file exist */
                if ( (rc = iotp_utils_fileExist(config->auth->privateKey)) != IoTP_SUCCESS ) {
                    LOG(ERROR, "Invalid device privateKey (%s) is specified: rc=%d", config->auth->privateKey, rc);
                    return rc;
                }
            }
        }
    }
        
    /* Validate application related config items */
    if ( type == IoTPClient_application ) {
        /* appiId, authToken and APIKey can not be empty */
        if (config->identity->appId == NULL || ( config->identity->appId && *config->identity->appId == '\0')) {
            rc = IoTP_RC_PARAM_NULL_VALUE;
            LOG(ERROR, "Application configuration appId is NULL or empty: rc=%d", rc);
            return rc;
        }
        if (config->auth->token == NULL || ( config->auth->token && *config->auth->token == '\0')) {
            rc = IoTP_RC_PARAM_NULL_VALUE;
            LOG(ERROR, "Application configuration token is NULL or empty: rc=%d", rc);
            return rc;
        }
        if (config->auth->apiKey == NULL || ( config->auth->apiKey && *config->auth->apiKey == '\0')) {
            rc = IoTP_RC_PARAM_NULL_VALUE;
            LOG(ERROR, "Application configuration APIKey is NULL or empty: rc=%d", rc);
            return rc;
        }
    }

    return rc;
}

/* Callback function to process successful disconnection */
void onDisconnect(void *context, MQTTAsync_successData5 *response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = client->clientId;
    LOG(INFO, "Client (id=%s) is successfully disconnected.", clientId? clientId:"NULL");
    client->connected = 0;
}

/* Callback function to process successful connection */
void onConnect(void *context, MQTTAsync_successData5 *response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = client->clientId;
    LOG(INFO, "Client (id=%s) is connected successfully.", clientId? clientId:"NULL");
    client->connected = 1;
}

/* Callback function to process connection failure */
void onConnectFailure(void* context, MQTTAsync_failureData5 *response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = NULL;
    if ( client && client->clientId )
        clientId = client->clientId;
    LOG(INFO, "Client (id=%s) connection has failed. rc=%d", clientId? clientId:"NULL", response? response->code:0);
    client->connected = -1;
}

/* Callback function to process successful send */
void onSend(void *context, MQTTAsync_successData5 *response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = client->clientId;
    LOG(DEBUG, "Event is sent from client (id=%s).", clientId? clientId:"NULL");
}

/* Callback function to process send failure */
void onSendFailure(void *context, MQTTAsync_failureData5 *response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = client->clientId;
    LOG(WARN, "Failed to send event from client (id=%s). rc=%d", clientId? clientId:"NULL", response? response->code:0);
}

/* Callback function to process successful subscription */
void onSubscribe(void* context, MQTTAsync_successData5 *response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = client->clientId;
    LOG(INFO, "Client (id=%s) has subscribe to the topic", clientId? clientId:"NULL");
}

/* Callback function to process subscription failure */
void onSubscribeFailure(void* context, MQTTAsync_failureData5 *response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = client->clientId;
    LOG(WARN, "Client (id=%s) has failed to subscribe to the topic: rc=%d", clientId? clientId:"NULL", response? response->code:0);
}

/* Callback function to process successful unsubscribed a topic */
void onUnSubscribe(void* context, MQTTAsync_successData5 *response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = client->clientId;
    LOG(INFO, "Client (id=%s) has unsubscribe from the topic", clientId? clientId:"NULL");
}

/* Callback function to process failure from unsubscribe call */
void onUnSubscribeFailure(void* context, MQTTAsync_failureData5 *response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = client->clientId;
    LOG(WARN, "Client (id=%s) has failed to unsubscribe from the topic: rc=%d", clientId? clientId:"NULL", response? response->code:0);
}

/* Creates IoTPClient */
IoTP_RC iotp_client_create(void **iotpClient, IoTPConfig *config, IoTPClientType type)
{
    IoTP_RC rc = IoTP_SUCCESS;

    /* set connection URI */
    char *connectionURI = NULL;
    char *clientId = NULL;
    int len = 0;

    /* Check if client and config handles are valid */
    /* Validate client type */
    if ( type < 1 && type > IoTPClient_total )  {
        rc = IoTP_RC_PARAM_INVALID_VALUE;
        LOG(ERROR, "Invalid IoTP client type %d is specified: rc=%d", type, rc);
        return rc;
    }
    /* validate handle */
    if ( iotpClient == NULL || config == NULL ) {
        rc = IoTP_RC_INVALID_HANDLE;
        LOG(ERROR, "Invalid (%s) handle (%s) or config handle (%s): rc=%d", IoTPClient_names[type], iotpClient?"Invalid":"Valid", config?"Valid":"Invalid", rc);
        return rc;
    }
    /* Make sure handle is not created before */
    if ( *iotpClient != NULL ) {
        rc = IoTP_RC_INVALID_HANDLE;
        LOG(ERROR, "Device handle (%s) is already created: rc=%d", IoTPClient_names[type], rc);
        return rc;
    }

    /* Validate client configuration */
    rc = iotp_validate_config(type, config);
    if ( rc != IoTP_SUCCESS ) {
        LOG(ERROR, "Failed to validate client (type=%d) configuration: rc=%d", type, rc);
        return rc;
    }

    char *domain = config->domain;
    int port = config->mqttopts->port;
    char *orgId = config->identity->orgId;
    int orgIdLen = strlen(orgId);

    len = orgIdLen + strlen(domain) + 23;   /* 23 = 6 for ssl:// + 11 for .messaging. + 5 for :port + 1 */
    connectionURI = (char *)malloc(len);
    if ( port == 1883 ) {
        snprintf(connectionURI, len, "tcp://%s.messaging.%s:%d", orgId, domain, port);
    } else {
        snprintf(connectionURI, len, "ssl://%s.messaging.%s:%d", orgId, domain, port);
    }

    LOG(DEBUG, "OrganizationID: %s", orgId);
    LOG(DEBUG, "ConnectionURI: %s", connectionURI);
    LOG(DEBUG, "Port: %d", port);

    /* set client id */
    if ( type == IoTPClient_device || type == IoTPClient_managed_device ) {
        len = orgIdLen + strlen(config->identity->typeId) + strlen(config->identity->deviceId) + 5;
        clientId = malloc(len);
        snprintf(clientId, len, "d:%s:%s:%s", orgId, config->identity->typeId, config->identity->deviceId);
    } else if ( type == IoTPClient_gateway || type == IoTPClient_managed_gateway ) {
        len = orgIdLen + strlen(config->identity->typeId) + strlen(config->identity->deviceId) + 5;
        clientId = malloc(len);
        snprintf(clientId, len, "g:%s:%s:%s", orgId, config->identity->typeId, config->identity->deviceId);
    } else if ( type == IoTPClient_application ) {
        len = orgIdLen + strlen(config->identity->appId) + 4;
        clientId = malloc(len);
        snprintf(clientId, len, "a:%s:%s", orgId, config->identity->appId);
    } else if ( type == IoTPClient_Application ) {
        len = orgIdLen + strlen(config->identity->appId) + 4;
        clientId = malloc(len);
        snprintf(clientId, len, "A:%s:%s", orgId, config->identity->appId);
    }

    LOG(DEBUG, "ClientId: %s", clientId);

    IoTPClient *client = (IoTPClient *)calloc(1, sizeof(IoTPClient));
    client->type = type;
    client->config = (void *)config;
    client->clientId = clientId;
    client->connectionURI = connectionURI;
    client->mqttClient = NULL;
    client->handlers = (IoTPHandlers *) calloc(1, sizeof(IoTPHandlers));

    /* create MQTT Async client handle */
    MQTTAsync                   mqttClient;
    MQTTAsync_createOptions     create_opts = MQTTAsync_createOptions_initializer;

    /* Create MQTT Async client - by default use MQTT V5 */
    create_opts.MQTTVersion = MQTTVERSION_5;
    create_opts.sendWhileDisconnected = 1;

    rc = MQTTAsync_createWithOptions(&mqttClient, client->connectionURI, client->clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL, &create_opts);
    if ( rc != MQTTASYNC_SUCCESS ) {
        LOG(ERROR, "MQTTAsync_createWithOptions failed: ClientType=%d ClientId=%s ConnectionURI=%s rc=%d",
            client->type, client->clientId, client->connectionURI, rc);
        iotp_client_destroy(client, 0);
        client = NULL;
        return rc;
    }

    /* add some delay for MQTT Client to get created */
    iotp_utils_delay(50);

    client->mqttClient = (void *)mqttClient;
    client->inited = 1;

    /* set client in IoTP client structure */
    *iotpClient = client;

    return rc;
}

/* Sets MQTT log handler for IoTP client */
IoTP_RC iotp_client_setMQTTLogHandler(void *iotpClient, IoTPLogHandler *cb) 
{
    IoTP_RC rc = IoTP_SUCCESS;
    IoTPClient *client = (IoTPClient *)iotpClient;

    /* Check if client handle is valid */
    if ( client == NULL || (client && client->config == NULL)) {
        rc = IoTP_RC_INVALID_HANDLE;
        return rc;
    } 

    IoTPConfig *config = (IoTPConfig *)client->config;

    /* set MQTTAsync trace callback */
    if ( config->logLevel > 0 ) {
        if ( config->mqttopts->traceLevel == 0 ) {
            if ( config->logLevel == LOGLEVEL_ERROR ) {
                MQTTAsync_setTraceLevel(MQTTASYNC_TRACE_FATAL);
                MQTTAsync_setTraceCallback((MQTTAsync_traceCallback *)cb);
            } else if ( config->logLevel == LOGLEVEL_WARN ) {
                MQTTAsync_setTraceLevel(MQTTASYNC_TRACE_SEVERE);
                MQTTAsync_setTraceCallback((MQTTAsync_traceCallback *)cb);
            } else if ( config->logLevel == LOGLEVEL_INFO ) {
                MQTTAsync_setTraceLevel(MQTTASYNC_TRACE_ERROR);
                MQTTAsync_setTraceCallback((MQTTAsync_traceCallback *)cb);
            } else if ( config->logLevel == LOGLEVEL_DEBUG ) {
                MQTTAsync_setTraceLevel(MQTTASYNC_TRACE_MAXIMUM);
                MQTTAsync_setTraceCallback((MQTTAsync_traceCallback *)cb);
            }
        } else {
            MQTTAsync_setTraceLevel(config->mqttopts->traceLevel);
            MQTTAsync_setTraceCallback((MQTTAsync_traceCallback *)cb);
        }
    }

    return rc;
}
 

/* Destroy IoTP Async client */
IoTP_RC iotp_client_destroy(void *iotpClient, int destroyMQTTClient)
{
    IoTP_RC rc = IoTP_SUCCESS;
    IoTPClient *client = (IoTPClient *)iotpClient;
    IoTPHandlers *handlers = NULL;
    int i = 0;

    /* Check if client handle is valid */
    if ( client == NULL || (client && client->mqttClient == NULL)) {
        rc = IoTP_RC_INVALID_HANDLE;
        return rc;
    } 

    /* if client is connected, disconnect and destory */
/*
    if ( iotp_client_isConnected(iotpClient) == IoTP_SUCCESS ) {
        MQTTAsync *mqttClient = (MQTTAsync *)client->mqttClient;
        iotp_client_disconnect(iotpClient);
        if ( destroyMQTTClient == 1 && mqttClient != NULL ) {
            MQTTAsync_destroy(mqttClient);
        }
    }
*/

    iotp_utils_freePtr((void *)client->clientId);
    iotp_utils_freePtr((void *)client->connectionURI);
    handlers = client->handlers;

    for (i=0; i<handlers->count; i++)
    {
        IoTPHandler * sub = handlers->entries[i];
        iotp_utils_freePtr((void *)sub->topic);
    }

    iotp_utils_freePtr((void *)handlers);

    /* set client config to NULL - so that config object is not affected */
    client->config = NULL;
    iotp_utils_freePtr((void *)client);
    client = NULL;

    return rc;
}

/* Connect MQTT Async client */
IoTP_RC iotp_client_connect(void *iotpClient)
{
    IoTP_RC rc = IoTP_SUCCESS;
    IoTPClient *client = (IoTPClient *)iotpClient;

    /* Sanity check */
    if ( client == NULL || (client && client->mqttClient == NULL)) {
        rc = IoTP_RC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle: rc=%d", rc);
        return rc;
    }

    IoTPConfig *config = (IoTPConfig *)client->config;
    if ( config == NULL ) {
        rc = IoTP_RC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client config handle: rc=%d", rc);
        return rc;
    }

    MQTTAsync_connectOptions    conn_opts = MQTTAsync_connectOptions_initializer;
    MQTTAsync_SSLOptions        ssl_opts = MQTTAsync_SSLOptions_initializer;
    MQTTProperty                property;
    MQTTProperties              props = MQTTProperties_initializer;

    /* MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer; */
    /* MQTTAsync_willOptions       will_opts = MQTTAsync_willOptions_initializer; */

    int port = config->mqttopts->port;

    /* set connection options */
    conn_opts.keepAliveInterval = config->mqttopts->keepalive;
    conn_opts.cleanstart = config->mqttopts->cleanStart;
    conn_opts.cleansession = 0;
    conn_opts.onSuccess5 = onConnect;
    conn_opts.onFailure5 = onConnectFailure;
    conn_opts.MQTTVersion = MQTTVERSION_5;
    conn_opts.context = client;
    conn_opts.automaticReconnect = config->automaticReconnect;
    if ( config->mqttopts->sessionExpiry > 0 ) {
        property.identifier = MQTTPROPERTY_CODE_SESSION_EXPIRY_INTERVAL;
        property.value.integer4 = config->mqttopts->sessionExpiry;
        MQTTProperties_add(&props, &property);
        conn_opts.connectProperties = &props;
    }

    /* set authentication credentials */
    ssl_opts.enableServerCertAuth = 0;

    if ( port == 8883 || port == 443 ) {
        /* set ssl */
        conn_opts.ssl = &ssl_opts;

        if ( client->type == IoTPClient_application || client->type == IoTPClient_Application ) {
            conn_opts.username = config->auth->apiKey;
            conn_opts.password = config->auth->token;
        } else if ( client->type == IoTPClient_device || client->type == IoTPClient_managed_device ||
            client->type == IoTPClient_gateway || client->type == IoTPClient_managed_gateway ) {
            if ( config->auth->token ) {
                conn_opts.username = "use-token-auth";
                conn_opts.password = config->auth->token;
            }
            if ( config->auth->keyStore ) {
                conn_opts.ssl->enableServerCertAuth = 1;
                conn_opts.ssl->trustStore = config->mqttopts->caFile;
                conn_opts.ssl->keyStore = config->auth->keyStore;
                conn_opts.ssl->privateKey = config->auth->privateKey;
                conn_opts.ssl->privateKeyPassword = config->auth->privateKeyPassword;
            }
        }
    }
    
    /* Set callbacks */
    MQTTAsync_setCallbacks((MQTTAsync *)client->mqttClient, (void *)client, NULL, iotp_client_messageArrived, NULL);
           
    if ((rc = MQTTAsync_connect((MQTTAsync *)client->mqttClient, &conn_opts)) == MQTTASYNC_SUCCESS) {
        int cycle = 0;
        LOG(INFO, "MQTTAsync_connect is called: ClientType=%d ClientId=%s  ConnectionURI=%s", 
            client->type, client->clientId, client->connectionURI);

        /* wait till client is disconnected */
        while ( client->connected == 0 ) {
            iotp_utils_delay(3000);
            if ( client->connected == -1 ) {
                LOG(ERROR, "Connection failed");
                client->connected = 0;
                break;
            }
            LOG(INFO, "Wait for client to connect: cycle=%d", cycle);
            if ( cycle > 40 )  {
                rc = IoTP_RC_TIMEOUT;
                break;
            }
            cycle++;
        }

    } else {
        LOG(INFO, "MQTTAsync_connect returned error: ClientType=%d ClientId=%s  ConnectionURI=%s rc=%d", 
            client->type, client->clientId, client->connectionURI, rc);
    }

    return rc;
}


/* Function used to publish the given data to the topic with the given QoS */
IoTP_RC iotp_client_publish(void *iotpClient, char *topic, char *payload, int qos, MQTTProperties *props)
{
    IoTP_RC rc = IoTP_SUCCESS;
    IoTPClient *client = (IoTPClient *)iotpClient;

    /* Sanity check */
    if ( !client || !client->config ) {
        rc = IoTP_RC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle: rc=%d", rc);
        return rc;
    }

    /* if client is not connected, return error */
    if ( client->connected == 0 ) {
        rc = IoTP_RC_NOT_CONNECTED;
        LOG(ERROR, "Failed to publish. Client is not coeecnted: rc=%d", rc);
        return rc;
    }

    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    MQTTAsync mqttClient = (MQTTAsync *)client->mqttClient;

    opts.onSuccess5 = onSend;
    opts.onFailure5 = onSendFailure;
    opts.context = client;
    if (props != NULL) {
        opts.properties = *props;
    }

    int payloadlen = 0;
    if ( payload && *payload != '\0' )
        payloadlen = strlen(payload);

    pubmsg.payload = payload;
    pubmsg.payloadlen = payloadlen;
    pubmsg.qos = qos;
    pubmsg.retained = 0;

    LOG(DEBUG, "Publish Message: qos=%d retained=%d payloadlen=%d payload: %s",
                    pubmsg.qos, pubmsg.retained, pubmsg.payloadlen, payload);

    rc = MQTTAsync_send(mqttClient, topic, payloadlen, payload, qos, 0, &opts);
    if ( rc != MQTTASYNC_SUCCESS && rc != IoTP_RC_INVALID_HANDLE ) {
        LOG(ERROR, "MQTTAsync_send returned error: rc=%d", rc);
        IoTPConfig *config = client->config;
        if ( config->automaticReconnect == 1 ) {
            LOG(WARN, "Connection lost, retry connection and republish message.\n");
            iotp_client_retry_connection(mqttClient);
            rc = MQTTAsync_send(mqttClient, topic, payloadlen, payload, qos, 0, &opts);
        }
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
        LOG(ERROR, "Invalid client handle: rc=%d", rc);
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

    opts.onSuccess5 = onSubscribe;
    opts.onFailure5 = onSubscribeFailure;
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

    opts.onSuccess5 = onUnSubscribe;
    opts.onFailure5 = onUnSubscribeFailure;
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
    if (client == NULL || (client && client->config == NULL)) {
        rc = IoTP_RC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle: rc=%d", rc);
        return rc;
    }

    if ( MQTTAsync_isConnected(client->mqttClient) == 0 ) {
        rc = IoTP_RC_NOT_CONNECTED; 
    }

    return rc;
}

/* Set the Handler (callback function). This must be set if you want to recieve commands */
IoTP_RC iotp_client_setHandler(void *iotpClient, char *topic, int type, IoTPCallbackHandler handler)
{
    IoTP_RC rc = 0;
    IoTPClient *client = (IoTPClient *)iotpClient;

    /* Sanity check */
    if (client == NULL || (client && client->config == NULL)) {
        rc = IoTP_RC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle: rc=%d", rc);
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
    if (client == NULL || (client && client->config == NULL)) {
        rc = IoTP_RC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle: rc=%d", rc);
        return rc;
    }

    /* Get callback */
    if ( client->handlers->count == 0 ) {
        /* no callback is configured */
        LOG(DEBUG, "Message Received on topic: %s", topicName? topicName:"");        
        LOG(DEBUG, "Message handler is not confogured. Client can not process received messages.");
        return rc;
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
            LOG(DEBUG, "Message Received on topic: %s", topicName? topicName:"");        
            LOG(DEBUG, "Message handler is not configured. Client can not process received messages.");
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

        LOG(DEBUG, "Calling registered callabck to process the arrived message");

        (*cb)(type,id,commandName, format, payload,payloadlen);

        MQTTAsync_freeMessage(&message);
        MQTTAsync_free(topicName);
    } else {
        LOG(DEBUG, "No registered callback function to process the arrived message");
    }

    return 1;
}


/* Disconnect from the IBM Watson IoT service */
IoTP_RC iotp_client_disconnect(void *iotpClient)
{
    IoTP_RC rc = 0;
    IoTPClient *client = (IoTPClient *)iotpClient;
    int cycle = 0;

    /* Sanity check */
    if (client == NULL || (client && client->config == NULL)) {
        rc = IoTP_RC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle: rc=%d", rc);
        return rc;
    }

    MQTTAsync mqttClient = (MQTTAsync *)client->mqttClient;
    MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;

    opts.onSuccess5 = onDisconnect;
    opts.context = client;

    if ( client->connected == 1 ) {
        LOG(INFO, "Disconnect client");
        int mqttRC = 0;
        mqttRC = MQTTAsync_disconnect(mqttClient, &opts);
        if ( mqttRC == MQTTASYNC_DISCONNECTED ) {
            rc = IoTP_SUCCESS;
        } else {
            rc = mqttRC;
           return rc;
        }

        /* wait till client is disconnected */
        while ( client->connected == 1 ) { 
            iotp_utils_delay(3000);
            LOG(INFO, "Wait for client to disconnect: cycle=%d", cycle);
            if ( cycle > 40 )  {
                rc = IoTP_RC_TIMEOUT;
                break;
            }
            cycle++;
        }
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
    IoTPClient *client = (IoTPClient *)iotpClient;
    int retry = 1;

    /* Sanity check */
    if (client == NULL || (client && client->config == NULL)) {
        rc = IoTP_RC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle: rc=%d", rc);
        return rc;
    }

    while((rc = iotp_client_connect(iotpClient)) != MQTTASYNC_SUCCESS)
    {
        LOG(DEBUG, "Retry Attempt #%d ", retry);
        int delay = reconnect_delay(retry++);
        LOG(DEBUG, " next attempt in %d seconds\n", delay);
        sleep(delay);
    }

    return rc;
}


