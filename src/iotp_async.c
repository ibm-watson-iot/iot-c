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
 * 
 * Contrinutors:
 *    Ranjan Dasgupta         - Initial drop
 *
 *******************************************************************************/

#include <MQTTAsync.h>

#include "iotp_utils.h"
#include "iotp_internal.h"
#include "Thread.h"

#if defined(WIN32) || defined(WIN64)
static mutex_type iotp_client_mutex = NULL;
static mutex_type iotp_managed_mutex = NULL;
#else
static pthread_mutex_t iotp_client_mutex_store = PTHREAD_MUTEX_INITIALIZER;
static mutex_type iotp_client_mutex = &iotp_client_mutex_store;
static pthread_mutex_t iotp_managed_mutex_store = PTHREAD_MUTEX_INITIALIZER;
static mutex_type iotp_managed_mutex = &iotp_managed_mutex_store;
#endif
static int iotp_mutex_inited = 0;

/*
 * Structure with DM action type and topic mapping 
 */
static struct {
    IoTP_DMAction_type_t  type;
    const char *          topic;
} dmActionTopics[] = {
    { 0,                        "NotUsed" },
    { IoTP_DMResponse,          NULL      },
    { IoTP_DMUpdate,            NULL      },
    { IoTP_DMObserve,           NULL      }, 
    { IoTP_DMCancel,            NULL      },
    { IoTP_DMFactoryReset,      NULL      },
    { IoTP_DMReboot,            NULL      },
    { IoTP_DMFirmwareDownload,  NULL      },
    { IoTP_DMFirmwareUpdate,    NULL      },
    { IoTP_DMActions,           NULL      }
};

#define numActionTopic  (sizeof(dmActionTopics)/sizeof(dmActionTopics[0]))

static int iotp_client_messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message * message);
static int iotp_client_dmMessageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message * message);


/* Initialize mutex - should be done only one time */
static void iotp_init_mutex(void)
{
    if ( iotp_mutex_inited == 1 ) {
        return;
    }

#if defined(WIN32) || defined(WIN64)
    if ( iotp_client_mutex == NULL ) {
        iotp_client_mutex = CreateMutex(NULL, 0, NULL);
        iotp_managed_mutex = CreateMutex(NULL, 0, NULL);
        iotp_mutex_inited = 1;
    }
#else
    int rc = 0;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    if ((rc = pthread_mutex_init(iotp_client_mutex, &attr)) != 0)
        LOG(ERROR, "Faild to initialize iotp_client_mutex. rc: %d", rc);
    if ((rc = pthread_mutex_init(iotp_managed_mutex, &attr)) != 0)
        LOG(ERROR, "Faild to initialize iotp_managed_mutex. rc: %d", rc);
    iotp_mutex_inited = 1;
#endif

}

/* Initialize DM Action Topics for managed device or managed gateway */
static IOTPRC iotp_init_dmActionTopics(IoTPClientType type, char *deviceType, char *deviceId)
{
    IOTPRC rc = IOTPRC_SUCCESS;
    int prefixLen = 0;
    int tlen = 0;
    char *prefix = NULL;
    char *topic = NULL;

    if ( type != IoTPClient_managed_device && type != IoTPClient_managed_gateway ) {
        rc = IOTPRC_INVALID_ARGS;
        LOG(ERROR, "Invalid action type is specified. type: %d | rc: %d", type, rc);
        return rc;
    }

    if ( type == IoTPClient_managed_device ) {
        prefixLen = strlen(DM_ACTION_DEVICE_PREFIXFMT) + 1;
        prefix = (char *) malloc(prefixLen);
        snprintf(prefix, prefixLen, DM_ACTION_DEVICE_PREFIXFMT);
    } else {
        if ( deviceType == NULL || deviceId == NULL ) {
            rc = IOTPRC_ARGS_NULL_VALUE;
            LOG(ERROR, "Invalid or NULL device type or id. deviceType: %s | deviceId: %s | rc: %d", deviceType?deviceType:"NULL", deviceId?deviceId:"NULL", rc);
            return rc;
        }

        prefixLen = strlen(DM_ACTION_GATEWAY_PREFIXFMT) + strlen(deviceType) + strlen(deviceId) + 1;
        prefix = (char *) malloc(prefixLen);
        snprintf(prefix, prefixLen, DM_ACTION_GATEWAY_PREFIXFMT, deviceType, deviceId);
    }

    tlen = strlen(DM_ACTION_RESPONSE);
    topic = (char *) malloc(prefixLen + tlen + 1);
    snprintf(topic, tlen, "%s%s", prefix, DM_ACTION_RESPONSE);
    dmActionTopics[IoTP_DMResponse].topic = topic;

    tlen = strlen(DM_ACTION_UPDATE);
    topic = (char *) malloc(prefixLen + tlen + 1);
    snprintf(topic, tlen, "%s%s", prefix, DM_ACTION_UPDATE);
    dmActionTopics[IoTP_DMResponse].topic = topic;

    tlen = strlen(DM_ACTION_OBSERVE);
    topic = (char *) malloc(prefixLen + tlen + 1);
    snprintf(topic, tlen, "%s%s", prefix, DM_ACTION_OBSERVE);
    dmActionTopics[IoTP_DMObserve].topic = topic;

    tlen = strlen(DM_ACTION_CANCEL);
    topic = (char *) malloc(prefixLen + tlen + 1);
    snprintf(topic, tlen, "%s%s", prefix, DM_ACTION_CANCEL);
    dmActionTopics[IoTP_DMCancel].topic = topic;

    tlen = strlen(DM_ACTION_FACTORYRESET);
    topic = (char *) malloc(prefixLen + tlen + 1);
    snprintf(topic, tlen, "%s%s", prefix, DM_ACTION_FACTORYRESET);
    dmActionTopics[IoTP_DMFactoryReset].topic = topic;

    tlen = strlen(DM_ACTION_REBOOT);
    topic = (char *) malloc(prefixLen + tlen + 1);
    snprintf(topic, tlen, "%s%s", prefix, DM_ACTION_REBOOT);
    dmActionTopics[IoTP_DMReboot].topic = topic;

    tlen = strlen(DM_ACTION_FIRMWAREDOWNLOAD);
    topic = (char *) malloc(prefixLen + tlen + 1);
    snprintf(topic, tlen, "%s%s", prefix, DM_ACTION_FIRMWAREDOWNLOAD);
    dmActionTopics[IoTP_DMFirmwareDownload].topic = topic;

    tlen = strlen(DM_ACTION_FIRMWAREUPDATE);
    topic = (char *) malloc(prefixLen + tlen + 1);
    snprintf(topic, tlen, "%s%s", prefix, DM_ACTION_FIRMWAREUPDATE);
    dmActionTopics[IoTP_DMFirmwareUpdate].topic = topic;

    tlen = strlen(DM_ACTION_ALL);
    topic = (char *) malloc(prefixLen + tlen + 1);
    snprintf(topic, tlen, "%s%s", prefix, DM_ACTION_ALL);
    dmActionTopics[IoTP_DMActions].topic = topic;

    return rc;
}

/* free DM action topics */
static void iotp_free_dmActionTopics(void)
{
    int i = 0;
    for (i=1; i<=IoTP_DMActions; i++) {
        if ( dmActionTopics[IoTP_DMActions].topic != NULL ) {
            free((void *)dmActionTopics[IoTP_DMActions].topic);
            dmActionTopics[IoTP_DMActions].topic = NULL;
        }
    }
}

/* Add callback handler to the handler list */
static IOTPRC iotp_add_handler(IoTPHandlers * handlers, IoTPHandler * handler) 
{
    IOTPRC rc = IOTPRC_SUCCESS;
    int i = 0;

    /* Initialize handlers table */
    if (handlers->count == handlers->nalloc) {
        IoTPHandler ** tmp = NULL;
        int firstSlot = handlers->nalloc;
        handlers->nalloc = handlers->nalloc == 0 ? 8 : handlers->nalloc * 2;
        tmp = realloc(handlers->entries, sizeof(IoTPHandler *) * handlers->nalloc);
        if (tmp == NULL) {
            return IOTPRC_NOMEM;
        }
        handlers->entries = tmp;
        for (i = firstSlot; i < handlers->nalloc; i++)
            handlers->entries[i] = NULL;
        handlers->slots = handlers->count;
        handlers->allCommandsId = 0;
        handlers->allDMActionsId = 0;
    }

    /* Add handler entry */
    if (handlers->count == handlers->slots) {
        handlers->entries[handlers->count] = handler;
        handlers->id = handlers->count;
        handlers->count++;
        handlers->slots++;
        if ( handler->type == IoTP_Handler_Commands ) handlers->allCommandsId = handlers->count;
        if ( handler->type == IoTP_Handler_DMActions ) handlers->allDMActionsId = handlers->count;
    } else {
        for (i = 0; i < handlers->slots; i++) {
            if (!handlers->entries[i]) {
                handlers->entries[i] = handler;
                handlers->id = i;
                handlers->count++;
                if ( handler->type == IoTP_Handler_Commands ) handlers->allCommandsId = handlers->count;
                if ( handler->type == IoTP_Handler_DMActions ) handlers->allDMActionsId = handlers->count;
                break;
            }
        }
    }

    return rc;
}

/* Find callback handler */
static IoTPHandler * iotp_client_getHandler(IoTPHandlers * handlers, char * topic) {
    int i = 0;
    IoTPHandler * handler = NULL;
    if (topic && strncmp(topic, COMMAND_ROOTTOPIC, COMMAND_ROOTTOPIC_LEN) == 0 && handlers->allCommandsId != 0 ) {
        handler = handlers->entries[handlers->allCommandsId - 1];
    } else if (topic && strncmp(topic, DM_ACTION_ROOTTOPIC, DM_ACTION_ROOTTOPIC_LEN) == 0 && handlers->allDMActionsId != 0 ) {
        handler = handlers->entries[handlers->allDMActionsId - 1];
    } else {
        int found = 0;
        for (i = 0; i < handlers->count; i++) {
            handler = handlers->entries[i];
            if (topic && handler->topic && strcmp(topic, handler->topic) == 0) {
                found = 1;
                break;
            }
        }
        if ( found == 0 ) handler = NULL;
    }
    return handler;
}

/* Returns handler type string */
static char * iotp_client_getHandlerTypeStr(IoTP_Handler_type_t type)
{
    switch(type) {
        case IoTP_Handler_Commands: return "Commands";
        case IoTP_Handler_Command: return "Command";
        case IoTP_Handler_Notification: return "Notification";
        case IoTP_Handler_MonitoringMessage: return "MonitoringMessage";
        case IoTP_Handler_DeviceMonitoring: return "DeviceMonitoring";
        case IoTP_Handler_AppMonitoring: return "AppMonitoring";
        case IoTP_Handler_AppEvent: return "AppEvent";
        default: return "NotSupported";
    }

    return "Unknown";
}


/* Returns DM action handler type string */
static char * iotp_client_getDMActionHandlerTypeStr(IoTP_DMAction_type_t type)
{
    switch(type) {
        case IoTP_DMResponse: return "DMResponse";
        case IoTP_DMUpdate: return "DMUpdate";
        case IoTP_DMObserve: return "DMObserve";
        case IoTP_DMCancel: return "DMCancel";
        case IoTP_DMFactoryReset: return "DMFactoryReset";
        case IoTP_DMReboot: return "DMReboot";
        case IoTP_DMFirmwareDownload: return "DMFirmwareDownload";
        case IoTP_DMFirmwareUpdate: return "DMFirmwareUpdate";
        case IoTP_DMActions: return "DMActions";
        default: return "NotSupported";
    }

    return "Unknown";
}

/* Validate client configuration */
static IOTPRC iotp_validate_config(int type, IoTPConfig *config)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* config, domain, or orgID */
    if ( !config ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        return rc;
    }

    /* domain and org ID can not be NULL */
    if ( config->domain == NULL || *config->domain == '\0' ) {
        rc = IOTPRC_PARAM_INVALID_VALUE;
        LOG(ERROR, "domain is NULL or empty");
        return rc;
    } else if ( config->identity->orgId == NULL || *config->identity->orgId == '\0' ) {
        if ( config->auth->apiKey == NULL || *config->auth->apiKey == '\0' ) {
            rc = IOTPRC_PARAM_INVALID_VALUE;
            LOG(ERROR, "APIKey is NULL or empty");
            return rc;
        } else {
            /* get orgid from API Key and update config */
            char *tmpstr = config->auth->apiKey;
            char *tok = strtok(tmpstr, "-");
            if ( tok != NULL ) {
                char *orgId = strtok(NULL, "-");
                config->identity->orgId = strdup(orgId);
            } else {
                LOG(WARN, "orgId is NULL or empty");
            }
        }
    }

    /* Check serverCertificatePath */
    /* Could be NULL for quickstart sandbox */
    if ( strcmp(config->domain, "quickstart") != 0 ) {
        if ( config->mqttopts->caFile == NULL || *config->mqttopts->caFile == '\0' ) {
            rc = IOTPRC_PARAM_INVALID_VALUE;
            LOG(WARN, "certificatePath is NULL or empty");
            return rc;
        }
        /* check if caFile is valid */
        if ( (rc = iotp_utils_fileExist(config->mqttopts->caFile)) != IOTPRC_SUCCESS ) {
            LOG(WARN, "caFile is NULL or empty");
            return rc;
        }
        /* check port - should be 8883 or 443 */
        if ( config->mqttopts->port != 8883 && config->mqttopts->port != 443 ) {
            rc = IOTPRC_PARAM_INVALID_VALUE;
            LOG(ERROR, "Invalid port. port: %d | rc: %d", config->mqttopts->port, rc);
            return rc;
        }
    } else {
        /* check port for quickstart - should be 1883 */
        if ( config->mqttopts->port != 1883 ) {
            rc = IOTPRC_PARAM_INVALID_VALUE;
            LOG(ERROR, "Invalid port for quickstart domain. port: %d | rc: %d", config->mqttopts->port, rc);
            return rc;
        }
    }

    /* Validate device or gateway related config items */
    if ( type == IoTPClient_device ) {
        /* device id and type can not be empty */
        if (config->identity->typeId == NULL || ( config->identity->typeId && *config->identity->typeId == '\0')) {
            rc = IOTPRC_PARAM_NULL_VALUE;
            LOG(ERROR, "typeId is NULL or empty");
            return rc;
        }
        if (config->identity->deviceId == NULL || ( config->identity->deviceId && *config->identity->deviceId == '\0')) {
            rc = IOTPRC_PARAM_NULL_VALUE;
            LOG(ERROR, "deviceId is NULL or empty");
            return rc;
        }
        /* validate device authMethod related config items */
        if ( config->authMethod == 0 ) {
            /* Should have valid authToken */
            if (config->auth->token == NULL || (config->auth->token && *config->auth->token == '\0')) {
                rc = IOTPRC_PARAM_NULL_VALUE;
                LOG(ERROR, "authToken is NULL or empty");
                return rc;
            }
        } else {
            /* should have valid client cert and key */
            if (config->auth->keyStore == NULL || ( config->auth->keyStore && *config->auth->keyStore == '\0')) {
                rc = IOTPRC_PARAM_NULL_VALUE;
                LOG(ERROR, "keyStore is NULL or empty");
                return rc;
            } else {
                /* check if file exist */
                if ( (rc = iotp_utils_fileExist(config->auth->keyStore)) != IOTPRC_SUCCESS ) {
                    LOG(ERROR, "Invalid keyStore is specified. keyStore: %s", config->auth->keyStore);
                    return rc;
                }
            }
            if (config->auth->privateKey == NULL || ( config->auth->privateKey && *config->auth->privateKey == '\0')) {
                rc = IOTPRC_PARAM_NULL_VALUE;
                LOG(ERROR, "privateKey is NULL or empty");
                return rc;
            } else {
                /* check if file exist */
                if ( (rc = iotp_utils_fileExist(config->auth->privateKey)) != IOTPRC_SUCCESS ) {
                    LOG(ERROR, "Invalid privateKey is specified. keyStore: %s", config->auth->privateKey);
                    return rc;
                }
            }
        }
    }
        
    /* Validate application related config items */
    if ( type == IoTPClient_application ) {
        /* appiId, authToken and APIKey can not be empty */
        if (config->identity->appId == NULL || ( config->identity->appId && *config->identity->appId == '\0')) {
            rc = IOTPRC_PARAM_NULL_VALUE;
            LOG(ERROR, "appId is NULL or empty");
            return rc;
        }
        if (config->auth->token == NULL || ( config->auth->token && *config->auth->token == '\0')) {
            rc = IOTPRC_PARAM_NULL_VALUE;
            LOG(ERROR, "token is NULL or empty");
            return rc;
        }
        if (config->auth->apiKey == NULL || ( config->auth->apiKey && *config->auth->apiKey == '\0')) {
            rc = IOTPRC_PARAM_NULL_VALUE;
            LOG(ERROR, "APIKey is NULL or empty");
            return rc;
        }
    }

    /* set log level */
    iotp_utils_setLogLevel(config->logLevel);

    return rc;
}

/* Callback function to process successful connection */
void onConnect(void *context, MQTTAsync_successData5 *response)
{
    char *clientId = NULL;
    IoTPClient *client = (IoTPClient *)context;
    Thread_lock_mutex(iotp_client_mutex);
    client->connected = 1;
    clientId = client->clientId;
    Thread_unlock_mutex(iotp_client_mutex);
    LOG(INFO, "Client is connected. clientId: %s", clientId? clientId:"NULL");
}

/* Callback function to process connection failure */
void onConnectFailure(void* context, MQTTAsync_failureData5 *response)
{
    char *clientId = NULL;
    int rc = 0;
    IoTPClient *client = (IoTPClient *)context;
    if ( response ) rc = response->code; 
    Thread_lock_mutex(iotp_client_mutex);
    client->connected = (0 - rc);
    clientId = client->clientId;
    Thread_unlock_mutex(iotp_client_mutex);
    if ( response ) {
        LOG(WARN, "Failed to connect. clientId: %s | rc: %d | respmsg: %s", clientId? clientId:"NULL", response->code, response->message?response->message:"");
    } else {
        LOG(WARN, "Failed to connect. clientId: %s | rc: | respmsg: ", clientId? clientId:"NULL");
    }
}

/* Callback function to process successful disconnection */
void onDisconnect(void *context, MQTTAsync_successData5 *response)
{
    char *clientId = NULL;
    IoTPClient *client = (IoTPClient *)context;
    Thread_lock_mutex(iotp_client_mutex);
    client->connected = 0;
    clientId = client->clientId;
    Thread_unlock_mutex(iotp_client_mutex);
    LOG(INFO, "Client is disconnected. clientId: %s", clientId? clientId:"NULL");
}

/* Callback function to process disconnection failure */
void onDisconnectFailure(void* context, MQTTAsync_failureData5 *response)
{
    char *clientId = NULL;
    int rc = 0;
    IoTPClient *client = (IoTPClient *)context;
    if ( response ) rc = response->code; 
    Thread_lock_mutex(iotp_client_mutex);
    client->connected = (0 - rc);
    clientId = client->clientId;
    Thread_unlock_mutex(iotp_client_mutex);
    if ( response ) {
        LOG(WARN, "Failed to disconnect. clientId: %s | rc: %d | respmsg: %s", clientId? clientId:"NULL", response->code, response->message?response->message:"");
    } else {
        LOG(WARN, "Failed to disconnect. clientId: %s | rc: | respmsg: ", clientId? clientId:"NULL");
    }
}

/* Callback function to process successful send */
void onSend(void *context, MQTTAsync_successData5 *response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = client->clientId;
    LOG(DEBUG, "Event is sent. clientId: %s", clientId? clientId:"NULL");
}

/* Callback function to process send failure */
void onSendFailure(void *context, MQTTAsync_failureData5 *response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = client->clientId;
    if ( response ) {
        LOG(WARN, "Failed to send event. clientId: %s | rc: %d | respmsg: %s", clientId? clientId:"NULL", response->code, response->message?response->message:"");
    } else {
        LOG(WARN, "Failed to send event. clientId: %s | rc: | respmsg: ", clientId? clientId:"NULL");
    }
}

/* Callback function to process successful subscription */
void onSubscribe(void* context, MQTTAsync_successData5 *response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = client->clientId;
    LOG(DEBUG, "Subscribe to a topic. clientId: %s", clientId? clientId:"NULL");
}

/* Callback function to process subscription failure */
void onSubscribeFailure(void* context, MQTTAsync_failureData5 *response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = client->clientId;
    if ( response ) {
        LOG(WARN, "Failed to subscribe. clientId: %s | rc: %d | respmsg: %s", clientId? clientId:"NULL", response->code, response->message?response->message:"");
    } else {
        LOG(WARN, "Failed to subscribe. clientId: %s | rc: | respmsg: ", clientId? clientId:"NULL");
    }
}

/* Callback function to process successful unsubscribed a topic */
void onUnSubscribe(void* context, MQTTAsync_successData5 *response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = client->clientId;
    LOG(INFO, "Unsubscribed from the topic. clientId: %s", clientId? clientId:"NULL");
}

/* Callback function to process failure from unsubscribe call */
void onUnSubscribeFailure(void* context, MQTTAsync_failureData5 *response)
{
    IoTPClient *client = (IoTPClient *)context;
    char *clientId = client->clientId;
    if ( response ) {
        LOG(WARN, "Failed to unsubscribe. clientId: %s | rc: %d | respmsg: %s", clientId? clientId:"NULL", response->code, response->message?response->message:"");
    } else {
        LOG(WARN, "Failed to unsubscribe. clientId: %s | rc: | respmsg: ", clientId? clientId:"NULL");
    }
}

/* Creates IoTPClient */
IOTPRC iotp_client_create(void **iotpClient, IoTPConfig *config, IoTPClientType type)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Initialize mutex */
    iotp_init_mutex();

    /* set connection URI */
    char *connectionURI = NULL;
    char *clientId = NULL;
    int len = 0;

    /* Validate client type */
    if ( type < 1 && type > IoTPClient_total )  {
        rc = IOTPRC_ARGS_INVALID_VALUE;
        LOG(ERROR, "Invalid client type is specified. type: %s", type);
        return rc;
    }

    /* validate handle */
    if ( iotpClient == NULL || config == NULL ) {
        rc = IOTPRC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client or config handle. type: %s  | client: %s | config: %s", IoTPClient_names[type], iotpClient?"Valid":"NULL", config?"Valid":"NULL");
        return rc;
    }

    /* Make sure handle is not created before */
    if ( *iotpClient != NULL ) {
        rc = IOTPRC_ARGS_INVALID_VALUE;
        LOG(ERROR, "Client handle is already created. type: %s", IoTPClient_names[type]);
        return rc;
    }

    /* Validate client configuration */
    rc = iotp_validate_config(type, config);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to validate configuration.");
        return rc;
    }

    /* Set client */
    char *domain = config->domain;
    int port = config->mqttopts->port;
    char *orgId = config->identity->orgId;
    int orgIdLen = strlen(orgId);

    len = orgIdLen + strlen(domain) + 24;   /* 24 = 6 for ssl:// + 11 for .messaging. + 6 for :port + 1 */
    connectionURI = (char *)malloc(len);
    if ( port == 1883 ) {
        snprintf(connectionURI, len, "tcp://%s.messaging.%s:%d", orgId, domain, port);
    } else {
        snprintf(connectionURI, len, "ssl://%s.messaging.%s:%d", orgId, domain, port);
    }

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
    } else if ( type == IoTPClient_Application ) {  /* A application - with shared subscription */
        len = orgIdLen + strlen(config->identity->appId) + 4;
        clientId = malloc(len);
        snprintf(clientId, len, "A:%s:%s", orgId, config->identity->appId);
    }

    LOG(INFO, "Create client. clientId: %s | connectionURI: %s | port: %d", clientId, connectionURI, port );

    IoTPClient *client = (IoTPClient *)calloc(1, sizeof(IoTPClient));
    client->type = type;
    client->config = (void *)config;
    client->clientId = clientId;
    client->connectionURI = connectionURI;
    client->mqttClient = NULL;
    client->handlers = (IoTPHandlers *) calloc(1, sizeof(IoTPHandlers));
    client->managed = 0;
    client->managedClient = NULL;

    /* Set Managed client fields */
    if ( type == IoTPClient_managed_device  || type == IoTPClient_managed_gateway ) {
        client->managedClient = (IoTPManagedClient *)calloc(1, sizeof(IoTPManagedClient));
    }

    /* create MQTT Async client handle */
    MQTTAsync                   mqttClient;
    MQTTAsync_createOptions     create_opts = MQTTAsync_createOptions_initializer;

    /* Create MQTT Async client - by default use MQTT V5 */
    create_opts.MQTTVersion = MQTTVERSION_5;
    create_opts.sendWhileDisconnected = 1;

    rc = MQTTAsync_createWithOptions(&mqttClient, client->connectionURI, client->clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL, &create_opts);
    if ( rc != MQTTASYNC_SUCCESS ) {
        LOG(ERROR, "MQTTAsync_createWithOptions failed. clientType: %d |  clientId: %s |  connectionURI: %s", client->type, client->clientId, client->connectionURI);
        iotp_client_destroy(client);
        client = NULL;
        return rc;
    }

    /* add some delay for MQTT Async client to get created */
    iotp_utils_delay(50);

    client->mqttClient = (void *)mqttClient;
    client->inited = 1;

    /* set client in IoTP client structure */
    *iotpClient = client;

    return rc;
}

/* Get device type from client handle */
char * iotp_client_getDeviceType(void *iotpClient)
{
    char *deviceType = NULL;
    IoTPClient *client = (IoTPClient *)iotpClient;

    /* Check if client handle is valid */
    if ( client == NULL || (client && client->config == NULL)) {
        return NULL;
    }

    IoTPConfig *config = (IoTPConfig *)client->config;
    if ( config->identity->typeId != NULL ) {
        deviceType = config->identity->typeId;
    }
    return deviceType;
}
    

/* Get device id from client handle */
char * iotp_client_getDeviceId(void *iotpClient)
{
    char *deviceId = NULL;
    IoTPClient *client = (IoTPClient *)iotpClient;

    /* Check if client handle is valid */
    if ( client == NULL || (client && client->config == NULL)) {
        return NULL;
    }

    IoTPConfig *config = (IoTPConfig *)client->config;
    if ( config->identity->deviceId != NULL ) {
        deviceId = config->identity->deviceId;
    }
    return deviceId;
}
    
/* Sets MQTT log handler for IoTP client */
IOTPRC iotp_client_setMQTTLogHandler(void *iotpClient, IoTPLogHandler *cb) 
{
    IOTPRC rc = IOTPRC_SUCCESS;
    IoTPClient *client = (IoTPClient *)iotpClient;

    /* Check if client handle is valid */
    if ( client == NULL || (client && client->config == NULL)) {
        rc = IOTPRC_INVALID_HANDLE;
        LOG(ERROR, "Invalid or NULL client handle or configuration");
        return rc;
    } 

    IoTPConfig *config = (IoTPConfig *)client->config;

    /* set MQTTAsync trace callback */
    /* Tracing is switched on by setting MQTT_C_CLIENT_TRACE. 
     * A value of ON, or stdout, prints to stdout, any other value is interpreted as a file name to use. 
     */
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
IOTPRC iotp_client_destroy(void *iotpClient)
{
    IOTPRC rc = IOTPRC_SUCCESS;
    IoTPClient *client = (IoTPClient *)iotpClient;
    IoTPHandlers *handlers = NULL;
    int i = 0;

    /* Check if client handle is valid */
    if ( client == NULL || (client && client->mqttClient == NULL)) {
        rc = IOTPRC_INVALID_HANDLE;
        LOG(ERROR, "Invalid or NULL client handle or configuration");
        return rc;
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

    /* set client config to NULL - so that config object is not affected */
    client->config = NULL;
    iotp_utils_freePtr((void *)client);
    client = NULL;

    return rc;
}

/* Connect MQTT Async client */
IOTPRC iotp_client_connect(void *iotpClient)
{
    IOTPRC rc = IOTPRC_SUCCESS;
    IoTPClient *client = (IoTPClient *)iotpClient;

    /* Sanity check */
    if ( client == NULL || (client && client->mqttClient == NULL)) {
        rc = IOTPRC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle");
        return rc;
    }

    IoTPConfig *config = (IoTPConfig *)client->config;
    if ( config == NULL ) {
        rc = IOTPRC_INVALID_HANDLE;
        LOG(ERROR, "NULL config handle");
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
            LOG(DEBUG, "key: %s | token: %s", config->auth->apiKey, config->auth->token);
        } else if ( client->type == IoTPClient_device || client->type == IoTPClient_managed_device ||
            client->type == IoTPClient_gateway || client->type == IoTPClient_managed_gateway ) {
            if ( config->auth->token ) {
                conn_opts.username = "use-token-auth";
                conn_opts.password = config->auth->token;
                LOG(DEBUG, "key: %s | token: %s", config->auth->apiKey, config->auth->token);
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
    
    /* Invoke MQTTAsync_connect */
    LOG(INFO, "MQTTAsync_connect. clientId=%s | connectionURI=%s", client->clientId, client->connectionURI);
    if ((rc = MQTTAsync_connect((MQTTAsync *)client->mqttClient, &conn_opts)) == MQTTASYNC_SUCCESS) {
        int cycle = 0;
        int isConnected = 0;
        while ( isConnected == 0 ) {
            iotp_utils_delay(3000);
            Thread_lock_mutex(iotp_client_mutex);
            isConnected =  client->connected;
            Thread_unlock_mutex(iotp_client_mutex);
            if (isConnected == 1) {
                break;
            }
            if ( isConnected < 0 ) {
                rc = (0 - isConnected);
                Thread_lock_mutex(iotp_client_mutex);
                client->connected = 0;
                Thread_unlock_mutex(iotp_client_mutex);
                break;
            }
            LOG(INFO, "Wait for client to connect. cycle=%d", cycle);
            if ( cycle > 40 )  {
                rc = IOTPRC_TIMEOUT;
                break;
            }
            cycle++;
        }

    }

    return rc;
}


/* Publishes message to a topic with specified QoS, and MQTTProperties */
IOTPRC iotp_client_publish(void *iotpClient, char *topic, char *payload, int qos, MQTTProperties *props)
{
    IOTPRC rc = IOTPRC_SUCCESS;
    IoTPClient *client = (IoTPClient *)iotpClient;

    /* Sanity check */
    if ( !client || !client->config ) {
        rc = IOTPRC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle");
        return rc;
    }

    /* if client is not connected, return error */
    if ( client->connected == 0 ) {
        rc = IOTPRC_NOT_CONNECTED;
        LOG(ERROR, "Not connected");
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

    LOG(DEBUG, "Publish event. topic: %s | qos: %d | retained: %d | payloadlen: %d | payload: %s",
                    topic, pubmsg.qos, pubmsg.retained, pubmsg.payloadlen, payload);

    rc = MQTTAsync_send(mqttClient, topic, payloadlen, payload, qos, 0, &opts);
    if ( rc != MQTTASYNC_SUCCESS && rc != IOTPRC_INVALID_HANDLE ) {
        LOG(ERROR, "MQTTAsync_send returned error: rc=%d", rc);
        IoTPConfig *config = client->config;
        if ( config->automaticReconnect == 1 ) {
            LOG(WARN, "Connection is lost, retry connection and republish message.");
            iotp_client_retry_connection(mqttClient);
            rc = MQTTAsync_send(mqttClient, topic, payloadlen, payload, qos, 0, &opts);
        }
    }

    return rc;
}

/* Subscribes to an MQTT topic to get command from WIoTP. */
IOTPRC iotp_client_subscribe(void *iotpClient, char *topic, int qos)
{
    IOTPRC rc = IOTPRC_SUCCESS;
    IoTPClient *client = (IoTPClient *)iotpClient;

    /* Sanity check */
    if ( !client || !client->config ) {
        rc = IOTPRC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle");
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

    LOG(DEBUG,"Subscribe. topic: %s | qos: %d", topic, qos);

    rc = MQTTAsync_subscribe(mqttClient, topic, qos, &opts);

    if ( rc != MQTTASYNC_SUCCESS ) {
        LOG(ERROR, "MQTTAsync_subscribe failed");
    }

    return rc;
}

/* Unsubscribe from an MQTT topic to get command from WIoTP. */
IOTPRC iotp_client_unsubscribe(void *iotpClient, char *topic)
{
    IOTPRC rc = IOTPRC_SUCCESS;
    IoTPClient *client = (IoTPClient *)iotpClient;

    MQTTAsync asyncClient = (MQTTAsync *)client->mqttClient;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;

    opts.onSuccess5 = onUnSubscribe;
    opts.onFailure5 = onUnSubscribeFailure;
    opts.context = asyncClient;

    LOG(DEBUG,"UnSubscribe. topic: %s", topic);

    rc = MQTTAsync_unsubscribe((MQTTAsync *)client->mqttClient, topic, &opts);
    if ( rc != MQTTASYNC_SUCCESS ) {
        LOG(ERROR, "MQTTAsync_unsubscribei failed");
    }

    return rc;
}

/* Checks if the client is connected */
IOTPRC iotp_client_isConnected(void *iotpClient)
{
    IOTPRC rc = IOTPRC_SUCCESS;
    IoTPClient *client = (IoTPClient *)iotpClient;

    /* Sanity check */
    if (client == NULL || (client && client->config == NULL)) {
        rc = IOTPRC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle");
        return rc;
    }

    if ( MQTTAsync_isConnected(client->mqttClient) == 0 ) {
        rc = IOTPRC_NOT_CONNECTED; 
    }

    return rc;
}

/* Sets the callback handler. This must be set if you want to recieve commands */
IOTPRC iotp_client_setHandler(void *iotpClient, char *topic, int type, IoTPCallbackHandler cbFunc)
{
    IOTPRC rc = IOTPRC_SUCCESS;
    IoTPClient *client = (IoTPClient *)iotpClient;

    /* Sanity check */
    if (client == NULL || (client && client->config == NULL)) {
        rc = IOTPRC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle");
        return rc;
    }

    /* for all received messages */
    if ( topic == NULL ) {
        topic = "iot-2/cmd/#";
    }

    /* Check if command handler is set for all commands */
    if ( client->handlers->allCommandsId != 0 ) {
        if ( type == IoTP_Handler_Commands ) {
            IoTPHandler * handler = client->handlers->entries[client->handlers->allCommandsId - 1];
            if ( handler->type == IoTP_Handler_Commands ) {
                handler->cbFunc = cbFunc;
                LOG(INFO, "Callback for all commands is updated.");
                return rc;
            } else {
                rc = IOTPRC_FAILURE;
                LOG(WARN, "Incorrect type to set callback for all commands. type: %d", handler->type);
                return rc;
            }
        } else {
            rc = IOTPRC_FAILURE;
            LOG(WARN, "Callback for all commands is already set. Can not set callback for topic: %s", topic);
            return rc;
        }
    }

    /* Loop thru all set callbacks, update or add */
    int found = 0;
    int i=0;
    for (i = 0; i < client->handlers->count; i++) {
        IoTPHandler *handler = client->handlers->entries[i];
        if (topic && handler->topic && strcmp(topic, handler->topic) == 0) {
            found = 1;
            break;
        }
    }
    if ( found == 0 ) {
        /* Add handler */
        IoTPHandler * handler = (IoTPHandler *)calloc(1, sizeof(IoTPHandler));
        handler->type = type;
        handler->topic = NULL;
        if ( topic && *topic != '\0' ) {
            handler->topic = strdup(topic);
        }
        handler->cbFunc = cbFunc;
        rc = iotp_add_handler(client->handlers, handler);
        if ( rc == IOTPRC_SUCCESS ) {
            LOG(INFO, "Handler (type=%s) is added. Topic=%s", iotp_client_getHandlerTypeStr(type), topic? topic:"NULL");
        } else {
            LOG(INFO, "Failed to add handler (type=%s) for topic=%s rc=%d", iotp_client_getHandlerTypeStr(type), topic? topic:"NULL", rc);
        }

    } else {
        /* Update handler */
        IoTPHandler * handler = client->handlers->entries[i];
        if ( handler->type == type ) {
            handler->cbFunc = cbFunc;
            LOG(INFO, "Callback is updated for topic: %s", topic);
        } else {
            rc = IOTPRC_FAILURE;
            LOG(INFO, "Callback update is requested for invalid type: %d topic=%s", type, topic);
        }
    }
    
    return rc;
}


/* Set the DM Action Handler - callback function */
IOTPRC iotp_client_setActionHandler(void *iotpClient, IoTP_DMAction_type_t type, IoTPDMActionHandler cbFunc)
{
    IOTPRC rc = 0;
    IoTPClient *client = (IoTPClient *)iotpClient;
    const char *topic;

    /* Sanity check */
    if (client == NULL || (client && client->config == NULL)) {
        rc = IOTPRC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle");
        return rc;
    }

    /* Get topic of the DM Action */
    if ( type < IoTP_DMResponse && type > IoTP_DMActions ) {
        rc = IOTPRC_HANDLER_INVALID;
        LOG(ERROR, "Invalid handle. type: %d", type);
        return rc;
    }

    /* Get topic string for the handle type */
    topic = dmActionTopics[type].topic;

    LOG(DEBUG, "Set DM Action callback. topic: %s", topic);

    /* Check if action handler is set for all DM actions */
    if ( client->handlers->allDMActionsId != 0 ) {
        if ( type == IoTP_Handler_DMActions ) {
            IoTPHandler * handler = client->handlers->entries[client->handlers->allDMActionsId - 1];
            if ( handler->type == IoTP_Handler_DMActions ) {
                handler->cbFunc = cbFunc;
                LOG(INFO, "Callback for all DM actions is updated.");
                return rc;
            } else {
                rc = IOTPRC_FAILURE;
                LOG(WARN, "Incorrect type to set callback for all DM actions. type: %d", handler->type);
                return rc;
            }
        } else {
            rc = IOTPRC_FAILURE;
            LOG(WARN, "Callback for all DM actions is already set. topic: %s", topic);
            return rc;
        }
    }

    /* Loop thru all set callbacks, update or add */
    int found = 0;
    int i=0;
    for (i = 0; i < client->handlers->count; i++) {
        IoTPHandler *handler = client->handlers->entries[i];
        if (topic && handler->topic && strcmp(topic, handler->topic) == 0) {
            found = 1;
            break;
        }
    }
    if ( found == 0 ) {
        /* Add handler to the list. */
        IoTPHandler * handler = (IoTPHandler *)calloc(1, sizeof(IoTPHandler));
        handler->type = type;
        handler->topic = NULL;
        if ( topic && *topic != '\0' ) {
            handler->topic = strdup(topic);
        }
        handler->cbFunc = cbFunc;
        rc = iotp_add_handler(client->handlers, handler);
        if ( rc == IOTPRC_SUCCESS ) {
            LOG(INFO, "Added handler. type: %s | topic: %s", iotp_client_getDMActionHandlerTypeStr(type), topic? topic:"NULL");
        } else {
            LOG(INFO, "Failed to add handler. type: %s | topic: %s", iotp_client_getDMActionHandlerTypeStr(type), topic? topic:"NULL");
        }
    } else {
        /* Update handler */
        IoTPHandler * handler = client->handlers->entries[i];
        if ( handler->type == type ) {
            handler->cbFunc = cbFunc;
            LOG(INFO, "Callback is updated. topic: %s", topic);
        } else {
            rc = IOTPRC_FAILURE;
            LOG(INFO, "Invalid type for callback update. type: %d | topic: %s", type, topic);
        }
    }

    return rc;
}


/* Handle received messages - invoke the callback. */
static int iotp_client_messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message * message)
{
    IOTPRC rc = IOTPRC_SUCCESS;
    IoTPClient *client = (IoTPClient *)context;

    if ( topicLen > 0 ) {
        LOG(DEBUG, "Message Received. topic: %s | topicLen: %d", topicName? topicName:"", topicLen);
    }

    /* sanity check */
    if (client == NULL || (client && client->config == NULL)) {
        rc = IOTPRC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle");
        goto msg_processed;
    }

    /* check for callbacks */
    if ( client->handlers->count == 0 ) {
        /* no callback is configured */
        rc = IOTPRC_HANDLER_NOT_FOUND;
        LOG(ERROR, "No callback is found");
        goto msg_processed;
    }

    /* Check if this message is from device management component */
    if ( topicName && strncmp(topicName, DM_ACTION_ROOTTOPIC, DM_ACTION_ROOTTOPIC_LEN) == 0 ) {
        int rc = iotp_client_dmMessageArrived(context, topicName, topicLen, message);
        return rc;
    }

    /* get callback */
    IoTPHandler * sub = iotp_client_getHandler(client->handlers, topicName);
    if ( sub == NULL ) {
        /* no callback is configured */
        rc = IOTPRC_HANDLER_NOT_FOUND;
        LOG(ERROR, "Callback not found for topic. topic: %s", topicName? topicName:"");
        goto msg_processed;
    }

    /* Processing gateway/device commands - Callback type should be greater than IoTP_Handler_Commands */
    if ( sub->type < IoTP_Handler_Commands ) {
        rc = IOTPRC_HANDLER_INVALID;
        goto msg_processed;
    }

    /* Set callback */
    IoTPCallbackHandler cb = (IoTPCallbackHandler)sub->cbFunc;

    /* Process incoming message if callback is defined */
    if (cb != 0) {
        char topic[4096];
        void *payload = message->payload;
        size_t payloadlen = message->payloadlen;
        char *type = NULL;
        char *id = NULL;
        char *commandName = NULL;
        char *format = NULL;
        char *pl = (char *)payload;

        snprintf(topic,4096, "%s", topicName);
        pl[payloadlen] = '\0';
        LOG(INFO, "Context: %x | Topic: %s | TopicLen: %d | PayloadLen: %d | Payload: %s", context, topic, topicLen, payloadlen, payload);
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

        LOG(DEBUG, "Invoke callabck to process message: cmd/evt: %s | format: %s", commandName, format);
        (*cb)(type, id, commandName, format, payload, payloadlen);
    } else {
        LOG(DEBUG, "No registered callback function is found to process the arrived message.");
    }

msg_processed:
    if ( rc == IOTPRC_SUCCESS ) 
        return 1;

    return 0;
}


/* Disconnect from the IBM Watson IoT service */
IOTPRC iotp_client_disconnect(void *iotpClient)
{
    IOTPRC rc = 0;
    IoTPClient *client = (IoTPClient *)iotpClient;
    int cycle = 0;

    /* Sanity check */
    if (client == NULL || (client && client->config == NULL)) {
        rc = IOTPRC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle");
        return rc;
    }

    MQTTAsync mqttClient = (MQTTAsync *)client->mqttClient;
    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;

    disc_opts.onSuccess5 = onDisconnect;
    disc_opts.onFailure5 = onDisconnectFailure;
    disc_opts.context = client;

    int isConnected = client->connected;
    if ( isConnected == 1 ) {
        LOG(INFO, "Disconnect client.");
        int mqttRC = 0;
        mqttRC = MQTTAsync_disconnect(mqttClient, &disc_opts);
        if ( mqttRC == MQTTASYNC_DISCONNECTED ) {
            rc = IOTPRC_SUCCESS;
        } else {
            rc = mqttRC;
            return rc;
        }

        Thread_lock_mutex(iotp_client_mutex);
        isConnected = client->connected;
        Thread_unlock_mutex(iotp_client_mutex);

        /* wait till client is disconnected */
        while ( isConnected == 1 ) {
            iotp_utils_delay(3000);
            Thread_lock_mutex(iotp_client_mutex);
            isConnected =  client->connected;
            Thread_unlock_mutex(iotp_client_mutex);
            if (isConnected == 0) {
                break;
            }
            if ( isConnected < 0 ) {
                rc = (0 - isConnected);
                Thread_lock_mutex(iotp_client_mutex);
                client->connected = 0;
                Thread_unlock_mutex(iotp_client_mutex);
                break;
            }
            LOG(INFO, "Wait for client to disconnect. cycle=%d", cycle);
            if ( cycle > 40 )  {
                rc = IOTPRC_TIMEOUT;
                break;
            }
            cycle++;
        }
    }

    /* Free DM action topics, if set*/
    iotp_free_dmActionTopics();

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
IOTPRC iotp_client_retry_connection(void *iotpClient)
{
    IOTPRC rc = IOTPRC_SUCCESS;
    IoTPClient *client = (IoTPClient *)iotpClient;
    int retry = 1;

    /* Sanity check */
    if (client == NULL || (client && client->config == NULL)) {
        rc = IOTPRC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle");
        return rc;
    }

    while((rc = iotp_client_connect(iotpClient)) != MQTTASYNC_SUCCESS)
    {
        int delay = reconnect_delay(retry++);
        LOG(DEBUG, "Connect retry. attempt: %d | delay: %d", retry, delay);
        iotp_utils_delay(1000*delay);
    }

    return rc;
}


/*
 * The following functions are related to device management.
 */

/* Sets device attributes */
IOTPRC iotp_client_setAttribute(void *iotpClient, char *name, char *value)
{
    IOTPRC rc = IOTPRC_SUCCESS;
    IoTPClient *client = (IoTPClient *)iotpClient;
    IoTPManagedClient *managedClient = client->managedClient;
  
    /* TODO - add code to get attribute and send request to platform */
    if ( !managedClient ) {
        rc = IOTPRC_INVALID_HANDLE;
        return rc;
    }

    if ( !name || !value ) {
        rc = IOTPRC_ARGS_INVALID_VALUE;
        return rc;
    }

    /* set lifetime */
    if ( name && strcasecmp(name, "lifetime") == 0 ) {
        int lifetime = atoi(value);
        if ( lifetime < 3600 ) lifetime = 0;
        managedClient->lifetime = lifetime;
    }
 
    /* set deviceActions */
    if ( name && strcasecmp(name, "deviceActions") == 0 ) {
        managedClient->supportsDeviceActions = atoi(value);
    }
 
    /* set firmwareActions */
    if ( name && strcasecmp(name, "firmwareActions") == 0 ) {
        managedClient->supportsFirmwareActions = atoi(value);
    }
 
    return rc;
} 

/* Send a device manage request to Watson IoT Platform Service */
IOTPRC iotp_client_manage(void *iotpClient)
{
    IOTPRC rc = IOTPRC_SUCCESS;
    IoTPClient *client = (IoTPClient *)iotpClient;
    IoTPManagedClient *managedClient = NULL;
    MQTTProperties *props = NULL;
    char uuid_str[64];
    char *payload;
    int plLen = 0;
    char *reqId = NULL;
    char *dInfo = NULL;
    char *mData = NULL;
    int reqIdLen = 0;
    int dInfoLen = 0;
    int mDataLen = 0;
    char *plFormat = "{\"d\":{\"metadata\":%s ,\"lifetime\":%ld,\"supports\":{\"deviceActions\":%d,\"firmwareActions\":%d},\"deviceInfo\":%s},\"reqId\":\"%s\"}";
    int fmLen = strlen(plFormat);

    /* verify handle */
    if ( !client || client->managedClient == NULL ) {
        rc = IOTPRC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle");
        return rc;
    }

    if ( client->managed == 1 ) {
        rc = IOTPRC_INVALID_HANDLE;
        LOG(ERROR, "Managed device is already initialized");
        return rc;
    }

    managedClient = client->managedClient;

    /* set request ID */
    if ( managedClient->reqID == NULL ) {
        iotp_utils_generateUUID(uuid_str);
        managedClient->reqID = strdup(uuid_str);
    }
    reqId = managedClient->reqID;
    reqIdLen = strlen(reqId);
    if ( managedClient->metadata != NULL ) {
        mData = managedClient->metadata;
    } else {
        mData = "{}";
    }
    mDataLen = strlen(mData);
    if ( managedClient->deviceInfo != NULL ) {
        dInfo = managedClient->deviceInfo;
    } else {
        dInfo = "{}";
    }
    dInfoLen = strlen(dInfo);

    plLen = fmLen + mDataLen + dInfoLen + reqIdLen + 10;
    payload = (char *)calloc(1, plLen);

    snprintf(payload, plLen, plFormat, mData, managedClient->lifetime, managedClient->supportsDeviceActions,
        managedClient->supportsFirmwareActions, dInfo, reqId);
 
    if ( client->type == IoTPClient_managed_gateway ) {
        char *deviceType = iotp_client_getDeviceType(client);
        char *deviceId = iotp_client_getDeviceId(client);

        /* Initialize DM action topics */
        iotp_init_dmActionTopics(IoTPClient_managed_gateway, deviceType, deviceId);

        rc = iotp_client_subscribe(iotpClient, "iotdm-1/#", QoS0);
        if ( rc == IOTPRC_SUCCESS ) {
            int prefixLen = strlen(DM_GATEWAY_TOPIC_PREFIXFMT) + strlen(deviceType) + strlen(deviceId) + 1;
            int pubtopicLen = prefixLen + strlen(DM_MANAGE) + 1;
            char prefix[prefixLen];
            char pubtopic[pubtopicLen];
            snprintf(prefix, prefixLen, DM_GATEWAY_TOPIC_PREFIXFMT, deviceType, deviceId); 
            snprintf(pubtopic, pubtopicLen, "%s%s", prefix, DM_MANAGE); 
            rc = iotp_client_publish(iotpClient, pubtopic, payload, QoS1, props);
            if ( rc == IOTPRC_SUCCESS ) {
                LOG(INFO, "Managed Gateway request sent. reqId: %s", reqId);
                client->managed = 1;
            } else {
                LOG(INFO, "Failed to send Managed Gateway request");
            }
        }

    } else {

        /* Initialize DM action topics */
        iotp_init_dmActionTopics(IoTPClient_managed_device, NULL, NULL);

        rc = iotp_client_subscribe(iotpClient, "iotdm-1/#", QoS0);
        if ( rc == IOTPRC_SUCCESS ) {
            int pubtopicLen = strlen(DM_DEVICE_TOPIC_PREFIXFMT) + strlen(DM_MANAGE) + 1;
            char pubtopic[pubtopicLen];
            snprintf(pubtopic, pubtopicLen, "%s%s", DM_DEVICE_TOPIC_PREFIXFMT, DM_MANAGE); 
            rc = iotp_client_publish(iotpClient, pubtopic, payload, QoS1, props);
            if ( rc == IOTPRC_SUCCESS ) {
                LOG(INFO, "Managed Device request sent. reqId: %s", reqId);
                client->managed = 1;
            } else {
                LOG(INFO, "Failed to send Managed Device request");
            }
        }
    }

    return rc;
}

/* Send an device unmanage request to Watson IoT Platform service */
IOTPRC iotp_client_unmanage(void *iotpClient, char *reqId)
{
    IOTPRC rc = IOTPRC_SUCCESS;
    IoTPClient *client = (IoTPClient *)iotpClient;
    IoTPManagedClient *managedClient = NULL;
    MQTTProperties *props = NULL;

    char uuid_str[40];
    char data[96];

    /* verify handle */
    if ( !client || client->managedClient == NULL ) {
        rc = IOTPRC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle");
        return rc;
    }

    if ( client->managed == 0 ) {
        rc = IOTPRC_INVALID_HANDLE;
        LOG(ERROR, "Managed client is not initialized");
        return rc;
    }

    managedClient = client->managedClient;

    /* set request ID */
    if ( reqId != NULL && *reqId != '\0' ) {
        snprintf(data, 96, "{\"reqId\":\"%s\"}", reqId);
        if ( managedClient->reqID != NULL ) {
            iotp_utils_freePtr((void *)managedClient->reqID);
        }
        managedClient->reqID = strdup(reqId);
    } else {
        if ( managedClient->reqID == NULL ) {
            iotp_utils_generateUUID(uuid_str);
            managedClient->reqID = strdup(uuid_str);
        }
        snprintf(data, 96, "{\"reqId\":\"%s\"}", managedClient->reqID);
    }

    rc = iotp_client_publish(iotpClient, DM_UNMANAGE, data, QoS0, props);
    if (rc == IOTPRC_SUCCESS) {
        LOG(DEBUG, "reqId = %s", managedClient->reqID);
        client->managed = 0;
    }

    return rc;
}

/* Get DM Action handler callback */
static IoTPDMActionHandler iotp_getActionCallback(IoTPClient *client, char *topicName) {
    IoTPDMActionHandler cb = NULL;

    /* check if callbacks are set */
    if ( client->handlers->count == 0 ) {
        /* no callback is configured */
        LOG(ERROR, "No callbacks are set for this client.");
        return NULL;
    }

    /* get DM Action callback */
    IoTPHandler * sub = iotp_client_getHandler(client->handlers, topicName);
    if ( sub == NULL ) {
        /* check if action handler for all actions are set */
        sub = iotp_client_getHandler(client->handlers, DM_ACTION_ALL);
        if ( sub == NULL ) {
            /* no callback is configured */
            LOG(ERROR, "Callback not found. topic: %s", topicName? topicName:"");
            return NULL;
        }
    }

    /* Callback type for DM actions should be less than IoTP_Handler_Commands */
    if ( sub->type >= IoTP_Handler_Commands ) {
        LOG(ERROR, "Invalid callback set. topic: %s", topicName? topicName:"");
        return NULL;
    }

    /* Set callback */
    cb = (IoTPDMActionHandler)sub->cbFunc;
    LOG(DEBUG, "Device Management action callback found for topic: %s", topicName? topicName:"");

    return cb;
}

/* Update device location */
static int iotp_updateLocationData(IoTPClient *client, char *reqID, int loc, int max, IoTP_json_parse_t *pobj)
{
    double latitude, longitude, elevation, accuracy;
    char* measuredDateTime;
    char* updatedDateTime;

    LOG(DEBUG,"Initiate update location data. reqID: %s", reqID);

    while ( loc <= max ) {
        IoTP_json_entry_t * ent = pobj->ent+loc;
        if ( ent->objtype == JSON_Object || ent->objtype == JSON_Array ) break;

        if (ent->name && !strcmp(ent->name, "latitude")) {
            latitude = strtod(ent->value, NULL);
        } else if (ent->name && !strcmp(ent->name, "longitude")) {
            longitude = strtod(ent->value, NULL);
        } else if (ent->name && !strcmp(ent->name, "elevation")) {
            elevation = strtod(ent->value, NULL);
        } else if (ent->name && !strcmp(ent->name, "accuracy")) {
            accuracy = strtod(ent->value, NULL);
        } else if (ent->name && !strcmp(ent->name, "measuredDateTime")) {
            measuredDateTime = (char *)ent->value;
        } else if (ent->name && !strcmp(ent->name, "updatedDateTime")) {
            updatedDateTime = (char *)ent->value;
        }
        loc++;
    }

    char data[1024];
    snprintf(data, 1024, "{\"d\":{\"longitude\":%f,\"latitude\":%f,\"elevation\":%f,\"measuredDateTime\":\"%s\",\"updatedDateTime\":\"%s\",\"accuracy\":%f},\"reqId\":\"%s\"}",
        latitude, longitude, elevation, measuredDateTime, updatedDateTime, accuracy, reqID);

    iotp_client_publish(client, DM_UPDATE_LOCATION, data, QoS1, NULL);
    return loc;
}


/* Update firmware data */
static int iotp_updateFirmwareData(IoTPClient *client, IoTPManagedClient *managedClient, char *reqID, int loc, int max, IoTP_json_parse_t *pobj)
{
    char response[128];

    while ( loc <= max ) {
        IoTP_json_entry_t * ent = pobj->ent+loc;
        if ( ent->objtype == JSON_Object || ent->objtype == JSON_Array ) break;

        if (ent->name && !strcmp(ent->name, "version")) {
            if ( managedClient->deviceFirmware.version ) free(managedClient->deviceFirmware.version);
            managedClient->deviceFirmware.version = ent->value? strdup(ent->value):NULL;
            LOG(DEBUG,"Firmware Version: %s", managedClient->deviceFirmware.version? managedClient->deviceFirmware.version:"");
        } else if (ent->name && !strcmp(ent->name, "name")) {
            if ( managedClient->deviceFirmware.name ) free(managedClient->deviceFirmware.name);
            managedClient->deviceFirmware.name = ent->value? strdup(ent->value):NULL;
            LOG(DEBUG,"Firmware Name: %s", managedClient->deviceFirmware.name? managedClient->deviceFirmware.name:"");
        } else if (ent->name && !strcmp(ent->name, "uri")) {
            if ( managedClient->deviceFirmware.uri ) free(managedClient->deviceFirmware.uri);
            managedClient->deviceFirmware.uri = ent->value? strdup(ent->value):NULL;
            LOG(DEBUG,"Firmware URI: %s", managedClient->deviceFirmware.uri? managedClient->deviceFirmware.uri:"");
        } else if (ent->name && !strcmp(ent->name, "verifier")) {
            if ( managedClient->deviceFirmware.verifier ) free(managedClient->deviceFirmware.verifier);
            managedClient->deviceFirmware.verifier = ent->value? strdup(ent->value):NULL;
            LOG(DEBUG,"Firmware Verifier: %s", managedClient->deviceFirmware.verifier? managedClient->deviceFirmware.verifier:"");
        } else if (ent->name && !strcmp(ent->name, "updatedDateTime")) {
            if ( managedClient->deviceFirmware.updatedDateTime ) free(managedClient->deviceFirmware.updatedDateTime);
            managedClient->deviceFirmware.updatedDateTime = ent->value? strdup(ent->value):NULL;
            LOG(DEBUG,"Firmware Update DateTime: %s", managedClient->deviceFirmware.updatedDateTime? managedClient->deviceFirmware.updatedDateTime:"");
        } else if (ent->name && !strcmp(ent->name, "state")) {
            managedClient->deviceFirmware.state = ent->count;
            LOG(DEBUG,"Firmware State: %d", managedClient->deviceFirmware.state);
        } else if (ent->name && !strcmp(ent->name, "updateStatus")) {
            managedClient->deviceFirmware.updateStatus = ent->count;
            LOG(DEBUG,"Firmware Update Status: %d", managedClient->deviceFirmware.updateStatus);
        }

        loc++;
    }

    sprintf(response, "{\"rc\":%d,\"reqId\":\"%s\"}", DM_ACTION_RC_UPDATE_SUCCESS, reqID);
    LOG(DEBUG,"Response: %s", response);

    iotp_client_publish(client, DM_RESPONSE, response, QoS1, NULL);

    return loc;
}

/* Handle received messages - invoke the callback. */
static int iotp_client_dmProcessReponse(IoTPClient *client, IoTPManagedClient *managedClient, char *topicName, int payloadlen, char *pl, IoTP_json_parse_t *pobj, char *reqID)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Get callback */
    IoTPDMActionHandler cb = iotp_getActionCallback(client, topicName);

    char *status = iotp_json_getString(pobj, "status");

    if ( cb != 0 ) {
        /* Invoke callback if returned request ID matches with request ID of the client */
        LOG(DEBUG, "Invoke registered callback. reqID: %s | status: %s", reqID, status?status:"");
        (*cb)(IoTP_DMResponse, (char *)reqID, pl, payloadlen);
    } else {
        rc = IOTPRC_DM_ACTION_NO_CALLBACK;
        LOG(WARN, "No registered callback found. reqID: %s | status: %s", reqID, status?status:"");
    }

    if ( rc == IOTPRC_SUCCESS )
        return 1;

    return 0;
}

/* Handle firmware download message */
static int iotp_client_dmProcessFirmwareDownload(IoTPClient *client, IoTPManagedClient *managedClient, char *topicName, int payloadlen, char *pl, IoTP_json_parse_t *pobj, char *reqID)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* check firmware state */
    if (managedClient != NULL && managedClient->deviceFirmware.state != FIRMWARESTATE_IDLE) {
        rc = DM_ACTION_RC_BAD_REQUEST;
        LOG(ERROR,"Device is not in the idle state");
        return rc;
    }

    /* Get callback */
    IoTPDMActionHandler cb = iotp_getActionCallback(client, topicName);

    LOG(DEBUG,"Initiate Firmware Download. reqID: %s", reqID);

    char respmsg[128];
    snprintf(respmsg, 128, "{\"rc\":%d,\"reqId\":%s}", DM_ACTION_RC_RESPONSE_ACCEPTED, reqID);
    iotp_client_publish(client, DM_RESPONSE, respmsg, QoS1, NULL);

    if ( cb != 0 ) {
        (*cb)(IoTP_DMFirmwareDownload, reqID, pl, payloadlen);
    } else {
        LOG(WARN, "Firmware download callback is not set.");
    }

    return rc;
}


/* Handle firmware update */
static int iotp_client_dmProcessFirmwareUpdate(IoTPClient *client, IoTPManagedClient *managedClient, char *topicName, int payloadlen, char *pl, IoTP_json_parse_t *pobj, char *reqID)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* check managed node firmware state */
    managedClient = client->managedClient;
    if (managedClient != NULL && managedClient->deviceFirmware.state != FIRMWARESTATE_DOWNLOADED) {
        rc = DM_ACTION_RC_BAD_REQUEST;
        LOG(ERROR,"The firmware image is not downloaded yet");
        return rc;
    }

    /* Get callback */
    IoTPDMActionHandler cb = iotp_getActionCallback(client, topicName);

    LOG(DEBUG, "Initiate Firmware Update. reqId: %s", reqID);

    char respmsg[128];
    snprintf(respmsg, 128, "{\"rc\":%d,\"reqId\":%s}", DM_ACTION_RC_RESPONSE_ACCEPTED, reqID);

    iotp_client_publish(client, DM_RESPONSE, respmsg, QoS1, NULL);

    if ( cb != 0 ) {
        (*cb)(IoTP_DMFirmwareUpdate, reqID, pl, payloadlen);
    } else {
        LOG(ERROR, "Firmware download callback is not set.");
    }

    return rc;
}


/* Handle update message */
static int iotp_client_dmProcessUpdate(IoTPClient *client, IoTPManagedClient *managedClient, char *topicName, int payloadlen, char *pl, IoTP_json_parse_t *pobj, char *reqID)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    LOG(DEBUG, "Initiate update. reqID: %s", reqID);

    int entnum = 0;
    int max = pobj->ent[entnum].count;
    int dpos = iotp_json_get(pobj, 0, "d");
    int fpos = iotp_json_get(pobj, dpos, "fields");

    /* Process update message */
    entnum = fpos+1;
    while ( entnum <= max ) {
        IoTP_json_entry_t * ent = pobj->ent+entnum;
        if ( ent->name && !strcmp("field", ent->name)) {
            if ( ent->value && !strcmp("location", ent->value)) {
                LOG(DEBUG,"Update Location.");
                entnum++;
                IoTP_json_entry_t * nent = pobj->ent+entnum;
                if ( nent->name && !strcmp("value", nent->name)) {
                    entnum++;
                    entnum = iotp_updateLocationData(client, reqID, entnum, max, pobj);
                }
            } else if ( ent->value && !strcmp("mgmt.firmware", ent->value)) {
                printf("Update firmware data.\n");
                entnum++;
                IoTP_json_entry_t * nent = pobj->ent+entnum;
                if ( nent->name && !strcmp("value", nent->name)) {
                    entnum++;
                    entnum = iotp_updateFirmwareData(client, managedClient, reqID, entnum, max, pobj);
                }
            } else {
                LOG(WARN,"Update is not supported. field: %s", ent->value? ent->value:"");
            }
        }
        entnum++;
    }

    return rc;
}

/* Handle device reset and rebbot */
static int iotp_client_dmProcessRebootReset(IoTPClient *client, char *topicName, int payloadlen, char *pl, char *reqID, int isReboot)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Get callback */
    IoTPDMActionHandler cb = iotp_getActionCallback(client, topicName);

    LOG(DEBUG, "Initiate reboot or reset. reqId: %s", reqID);

    if ( cb != 0 ) {
        if ( isReboot ) {
            LOG(DEBUG,"Invoking device reboot callback");
            (*cb)(IoTP_DMReboot, reqID, pl, payloadlen);
        } else {
            LOG(DEBUG,"Invoking device reset callback");
            (*cb)(IoTP_DMFactoryReset, reqID, pl, payloadlen);
        }
    } else {
        LOG(WARN, "Firmware download callback is not set.");
    }

    return rc;
}

/* Handle Observe action */
static int iotp_client_dmProcessObserve(IoTPClient *client, char *reqID)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    LOG(DEBUG, "Initiate observ. reqId: %s", reqID);

    char respmsg[256];
    char *plFormat = "{\"rc\":%d,\"reqId\":\"%s\",\"d\":{\"fields\":[{\"field\":\"mgmt.firmware\",\"value\":{\"state\":0,\"updateStatus\":0}}]}}";
    snprintf(respmsg, 256, plFormat, DM_ACTION_RC_RESPONSE_SUCCESS, reqID);
    iotp_client_publish(client, DM_RESPONSE, respmsg, QoS1, NULL);

    return rc;
}

/* Handle cancel action */
static int iotp_client_dmProcessCancel(IoTPClient *client, IoTPManagedClient *managedClient, IoTP_json_parse_t *pobj, char *reqID)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    LOG(DEBUG, "Initiate cancel. reqId: %s", reqID);

    int entnum = 0;
    int max = pobj->ent[entnum].count;
    int dpos = iotp_json_get(pobj, 0, "d");
    int fpos = iotp_json_get(pobj, dpos, "fields");

    /* Process payload */
    entnum = fpos+1;
    while ( entnum <= max ) {
        IoTP_json_entry_t * ent = pobj->ent+entnum;
        if ( ent->name && !strcmp("field", ent->name)) {
            if ( ent->value && !strcmp("mgmt.firmware", ent->value)) {
                LOG(DEBUG, "Reset managed client observe flag.");
                managedClient->observe = 0;
                char respmsg[128];
                sprintf(respmsg,"{\"rc\":%d,\"reqId\":%s}", DM_ACTION_RC_RESPONSE_SUCCESS, reqID);
                iotp_client_publish(client, DM_RESPONSE, respmsg, QoS1, NULL);
                break;
            }
        }

        entnum++;
    }

    return rc;
}


/* Handle received messages - invoke the callback. */
static int iotp_client_dmMessageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message * message)
{
    IOTPRC rc = IOTPRC_SUCCESS;
    IoTPClient *client = (IoTPClient *)context;
    IoTPManagedClient *managedClient = NULL;
    IoTP_json_parse_t *pobj = NULL;
    char *pl = NULL;

    Thread_lock_mutex(iotp_managed_mutex);

    /* sanity check */
    if (client == NULL || (client && client->config == NULL)) {
        rc = IOTPRC_INVALID_HANDLE;
        LOG(ERROR, "Invalid client handle");
        Thread_unlock_mutex(iotp_managed_mutex);
        return rc;
    }

    /* Get managedClient handle */
    managedClient = client->managedClient;
    if ( managedClient == NULL ) {
        rc = IOTPRC_INVALID_HANDLE;
        LOG(ERROR, "Not a managed client");
        Thread_unlock_mutex(iotp_managed_mutex);
        return rc;
    }

    /* Set JSON object */
    void *payload = message->payload;
    size_t payloadlen = message->payloadlen;
    pl = (char *) malloc(payloadlen+1);

    memset(pl, 0, payloadlen+1);
    strncpy(pl, payload, payloadlen);

    pobj = iotp_json_init(payloadlen, payload);
    if ( pobj == NULL ) {
        rc = IOTPRC_DM_RESPONSE_PARSE_ERROR;
        goto endDMAction;
    }

    /* Get request ID from payload */
    char *reqID = iotp_json_getString(pobj, "reqId");

    LOG(DEBUG, "Device Management request. reqID: %s", reqID?reqID:"");

    /* invoke DM action processing functions based on topic name */
    if (strstr(topicName, DM_ACTION_RESPONSE)) {
        if ( reqID == NULL ) {
            rc = IOTPRC_DM_RESPONSE_NULL_REQID;
            LOG(ERROR, "NULL reqID in response");
            goto endDMAction;
        }
        if ( managedClient && managedClient->reqID == NULL && reqID != NULL ) {
            managedClient->reqID = strdup(reqID);
        }
        rc = iotp_client_dmProcessReponse(client, managedClient, topicName, payloadlen, pl, pobj, reqID);
        goto endDMAction;
    }

    /* check if reqID is the current request ID set for this managedClient */
    if (managedClient && managedClient->reqID && !strcmp(managedClient->reqID, reqID)) {
        rc = IOTPRC_DM_RESPONSE_INVALID_REQID;
        LOG(ERROR, "Invalid request ID. reqID: %s |  expectedReqID: %s", reqID?reqID:"", managedClient->reqID? managedClient->reqID:"");
        goto endDMAction;
    } 

    if (strstr(topicName, DM_ACTION_FIRMWAREDOWNLOAD)) {
        iotp_client_dmProcessFirmwareDownload(client, managedClient, topicName, payloadlen, pl, pobj, reqID);
    } else if (strstr(topicName, DM_ACTION_FIRMWAREUPDATE)) {
        iotp_client_dmProcessFirmwareUpdate(client, managedClient, topicName, payloadlen, pl, pobj, reqID);
    } else if (strstr(topicName, DM_ACTION_UPDATE)) {
        iotp_client_dmProcessUpdate(client, managedClient, topicName, payloadlen, pl, pobj, reqID);
    } else if (strstr(topicName, DM_ACTION_REBOOT)) {
        iotp_client_dmProcessRebootReset(client, topicName, payloadlen, pl, reqID, 1);
    } else if (strstr(topicName, DM_ACTION_FACTORYRESET)) {
        iotp_client_dmProcessRebootReset(client, topicName, payloadlen, pl, reqID, 0);
    } else if (strstr(topicName, DM_ACTION_OBSERVE)) {
        iotp_client_dmProcessObserve(client, reqID);
    } else if (strstr(topicName, DM_ACTION_CANCEL)) {
        iotp_client_dmProcessCancel(client, managedClient, pobj, reqID);
    }

endDMAction:
    if (pobj) 
        iotp_json_free(pobj);
    if (pl) 
        free(pl);

    Thread_unlock_mutex(iotp_managed_mutex);
    return rc;
}



