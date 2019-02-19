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

#include "iotp_config.h"
#include "iotp_internal.h"

/* Array of IoTP Client names */
char * IoTPClient_names[IoTPClient_total] = {
    "Unknown",
    "Device",
    "Gateway",
    "Application",
    "ScalableApplication",
    "ManagedDevice",
    "ManagedGateway"
};

/* IoTPConfig_setLogHandler: Sets log handler */
IOTPRC IoTPConfig_setLogHandler(IoTPLogTypes type, void * handler)
{
    IOTPRC rc = IOTPRC_SUCCESS;
    rc = iotp_utils_setLogHandler(type, handler);
    return rc;
}

/* IoTPConfig_create: Creates IBM Watson IoT client configuration object */
IOTPRC IoTPConfig_create(IoTPConfig **config, const char * configFileName)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Check if config handle is valid i.e. not NULL or already inited */
    if ( config == NULL ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(ERROR, "NULL configuration object is specified: rc=%d", rc);
        return rc;
    }
    if ( config && *config != NULL ) {
        rc = IOTPRC_INVALID_HANDLE;
        LOG(ERROR, "Configuration object is already initialized: rc=%d", rc);
        return rc;
    }


    iotp_utils_writeClientVersion();
    LOG(INFO, "Create configuration object from file: %s", configFileName?configFileName:"");

    /* Create configuration handle */
    *config = (IoTPConfig *)calloc(1, sizeof(IoTPConfig));
    if ( *config == NULL ) {
        rc = IOTPRC_NOMEM;
        return rc;
    }

    /* Set identity parameters */
    identity_t *identity = (identity_t *)calloc(1, sizeof(identity_t));
    if ( identity == NULL ) {
        rc = IOTPRC_NOMEM;
        return rc;
    }

    /* Set auth parameters */
    auth_t *auth = (auth_t *)calloc(1, sizeof(auth_t));
    if ( auth == NULL ) {
        rc = IOTPRC_NOMEM;
        return rc;
    }

    /* Set mqttopts */
    mqttopts_t *mqttopts = (mqttopts_t *)calloc(1, sizeof(mqttopts_t));
    if ( mqttopts == NULL ) {
        rc = IOTPRC_NOMEM;
        return rc;
    }
    mqttopts->transport = strdup("tcp");
    mqttopts->port = 8883;
    mqttopts->caFile = strdup("./IoTPlatform.pem");
    mqttopts->cleanSession = 1;
    mqttopts->cleanStart = 0;
    mqttopts->keepalive = 60;
    mqttopts->sessionExpiry = 3600;
    mqttopts->sharedSubscription = 0;
    mqttopts->validateServerCert = 1;


#ifdef HTTP_IMPLEMENTED
    /* Set httpopts */
    httpopts_t *httpopts = (httpopts_t *)calloc(1, sizeof(httpopts_t));
    if ( httpopts == NULL ) {
        rc = IOTPRC_NOMEM;
        return rc;
    }
    httpopts->validateServerCert = 1;
#endif

    /* Set config */
    (*config)->domain = strdup("internetofthings.ibmcloud.com");
    (*config)->logLevel = LOGLEVEL_INFO;
    (*config)->type = IoTPClient_device;
    (*config)->identity = identity;
    (*config)->auth = auth;
    (*config)->mqttopts = mqttopts;

    (*config)->automaticReconnect = 1;    /* internal option */
    (*config)->authMethod = 0;            /* internal option */


#ifdef HTTP_IMPLEMENTED
    (*config)->httpopts = httpopts;
#endif

    /* If configuration file is specified - process it */
    if ( configFileName && *configFileName != '\0' ) {
        rc = IoTPConfig_readConfigFile(*config, configFileName);
    }

    return rc;
}


/* Clears all properties from a configuration object */
IOTPRC IoTPConfig_clear(IoTPConfig *config) 
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* sanity check */
    if (!config) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(ERROR, "NULL configuration object is specified: rc=%d", rc);
    } else {
        /* free config object */
        iotp_utils_freePtr((void *)config->domain);
        if ( config->identity != NULL ) {
            identity_t *identity = config->identity;
            iotp_utils_freePtr((void *)identity->orgId);
            iotp_utils_freePtr((void *)identity->typeId);
            iotp_utils_freePtr((void *)identity->deviceId);
            iotp_utils_freePtr((void *)identity->appId);
            iotp_utils_freePtr((void *)config->identity);
        }
        if ( config->auth != NULL ) {
            auth_t *auth = config->auth;
            iotp_utils_freePtr((void *)auth->keyStore);
            iotp_utils_freePtr((void *)auth->privateKey);
            iotp_utils_freePtr((void *)auth->privateKeyPassword);
            iotp_utils_freePtr((void *)auth->token);
            iotp_utils_freePtr((void *)auth->apiKey);
            iotp_utils_freePtr((void *)config->auth);
        }
        if ( config->mqttopts != NULL ) {
            mqttopts_t *mqttopts = config->mqttopts;
            iotp_utils_freePtr((void *)mqttopts->transport);
            iotp_utils_freePtr((void *)mqttopts->caFile);
            iotp_utils_freePtr((void *)config->mqttopts);
        }

#ifdef HTTP_IMPLEMENTED
        if ( config->httpopts != NULL ) {
            iotp_utils_freePtr((void *)config->httpopts);
        }
#endif

        iotp_utils_freePtr(config);
    }
    return rc;
}


/* IoTPConfig_setProperty: Set IoTP configuration object properties */
IOTPRC IoTPConfig_setProperty(IoTPConfig *config, const char * name, const char * value)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    char *argptr = NULL;
    int   argint = 0;

    /* sanity check */
    if (!config) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(ERROR, "NULL configuration object is specified: rc=%d", rc);
        return rc;
    }
    if (!name || *name == '\0') {
        rc = IOTPRC_INVALID_PARAM;
        LOG(ERROR, "Invalid configuration parameter is specified: rc=%d", rc);
        return rc;
    }

    argptr = (char *)value;
    if ( argptr && *argptr != '\0' ) {
        argint = atoi(argptr);
    }

    /* Process Identity configuration items */
    if ( !strncasecmp(name, "identity.", 9)) {
        /* Process identity.orgid */
        if ( !strcasecmp(name, IoTPConfig_identity_orgId)) {
            if (argptr == NULL || *argptr == '\0') {
                rc = IOTPRC_PARAM_NULL_VALUE;
            } else if (argint != 0 ) {
                rc = IOTPRC_PARAM_INVALID_VALUE;
            } else {
                int len = strlen(argptr);
                if ( len != 6 ) {
                    rc = IOTPRC_PARAM_INVALID_VALUE;
                } else {
                    if ( config->identity->orgId ) 
                        iotp_utils_freePtr((void *)config->identity->orgId);
                    config->identity->orgId = strdup(argptr);
                }
            }
            goto setPropDone;
        }
    
        /* Process identity.typeId */
        if ( !strcasecmp(name, IoTPConfig_identity_typeId)) {
            if (argptr == NULL || (argptr && *argptr == '\0')) {
                rc = IOTPRC_PARAM_NULL_VALUE;
            } else {
                if ( config->identity->typeId ) 
                    iotp_utils_freePtr((void *)config->identity->typeId);
                config->identity->typeId =  strdup(argptr);
            }
            goto setPropDone;
        }

        /* Process identity.deviceId */
        if ( !strcasecmp(name, IoTPConfig_identity_deviceId)) {
            if (argptr == NULL || (argptr && *argptr == '\0')) {
                rc = IOTPRC_PARAM_NULL_VALUE;
            } else {
                if ( config->identity->deviceId ) 
                    iotp_utils_freePtr((void *)config->identity->deviceId);
                config->identity->deviceId =  strdup(argptr);
            }
            goto setPropDone;
        }
    
        /* Process identity.appId */
        if ( !strcasecmp(name, IoTPConfig_identity_appId)) {
            if (argint != 0) {
                rc = IOTPRC_PARAM_INVALID_VALUE;
            } else if (argptr == NULL || *argptr == '\0') {
                rc = IOTPRC_PARAM_NULL_VALUE;
            } else {
                if ( config->identity->appId ) 
                    iotp_utils_freePtr((void *)config->identity->appId);
                config->identity->appId = strdup(argptr);
            }
            goto setPropDone;
        }
    }
    
    /* Process Auth configuration items */
    if ( !strncasecmp(name, "auth.", 5)) {
        /* Process auth.keyStore */
        if ( !strcasecmp(name, IoTPConfig_auth_keyStore)) {
            if (argint != 0) {
                rc = IOTPRC_PARAM_INVALID_VALUE;
            } else if (argptr == NULL || *argptr == '\0') {
                if ( config->auth->keyStore ) 
                    iotp_utils_freePtr((void *)config->auth->keyStore);
                config->auth->keyStore = strdup("./IoTFoundation.pem");
            } else {
                if ( config->auth->keyStore ) 
                    iotp_utils_freePtr((void *)config->auth->keyStore);
                config->auth->keyStore = strdup(argptr);
            }
            goto setPropDone;
        }

        /* Process auth.privateKey */
        if ( !strcasecmp(name, IoTPConfig_auth_privateKey)) {
            if (argint != 0) {
                rc = IOTPRC_PARAM_INVALID_VALUE;
            } else if (argptr == NULL || *argptr == '\0') {
                if ( config->auth->privateKey ) 
                    iotp_utils_freePtr((void *)config->auth->privateKey);
                config->auth->privateKey = NULL;
            } else {
                if ( config->auth->privateKey ) 
                    iotp_utils_freePtr((void *)config->auth->privateKey);
                config->auth->privateKey = strdup(argptr);
            }
            goto setPropDone;
        }

        /* Process auth.privateKeyPassword */
        if ( !strcasecmp(name, IoTPConfig_auth_privateKeyPassword)) {
            if (argint != 0) {
                rc = IOTPRC_PARAM_INVALID_VALUE;
            } else if (argptr == NULL || *argptr == '\0') {
                if ( config->auth->privateKeyPassword ) 
                    iotp_utils_freePtr((void *)config->auth->privateKeyPassword);
                config->auth->privateKeyPassword = NULL;
            } else {
                if ( config->auth->privateKeyPassword ) 
                    iotp_utils_freePtr((void *)config->auth->privateKeyPassword);
                config->auth->privateKeyPassword = strdup(argptr);
            }
            goto setPropDone;
        }

        /* Process auth.token */
        if ( !strcasecmp(name, IoTPConfig_auth_token)) {
            if (argint != 0) {
                rc = IOTPRC_PARAM_INVALID_VALUE;
            } else if (argptr == NULL || *argptr == '\0') {
                if ( config->auth->token ) 
                    iotp_utils_freePtr((void *)config->auth->token);
                config->auth->token = NULL;
            } else {
                if ( config->auth->token ) 
                    iotp_utils_freePtr((void *)config->auth->token);
                config->auth->token = strdup(argptr);
            }
            goto setPropDone;
        }

        /* Process auth.APIKey */
        if ( !strcasecmp(name, IoTPConfig_auth_APIKey)) {
            if (argint != 0) {
                rc = IOTPRC_PARAM_INVALID_VALUE;
            } else if (argptr == NULL || *argptr == '\0') {
                if ( config->auth->apiKey ) 
                    iotp_utils_freePtr((void *)config->auth->apiKey);
                config->auth->apiKey = NULL;
            } else {
                if ( config->auth->apiKey ) 
                    iotp_utils_freePtr((void *)config->auth->apiKey);
                config->auth->apiKey = strdup(argptr);
            }
            goto setPropDone;
        }

    }


    /* Process Optional configuration items */
    if ( !strncasecmp(name, "Options.", 8)) {
        /* Process options.domain */
        if ( !strcasecmp(name, IoTPConfig_options_domain)) {
            if (argint != 0) {
                rc = IOTPRC_PARAM_INVALID_VALUE;
                LOG(ERROR, "Invalid configuration value is specified: rc=%d", rc);
            } else if (argptr == NULL || *argptr == '\0') {
                if ( config->domain ) 
                    iotp_utils_freePtr((void *)config->domain);
                config->domain = strdup("internetofthings.ibmcloud.com");
            } else {
                if ( config->domain ) 
                    iotp_utils_freePtr((void *)config->domain);
                config->domain = strdup(argptr);
            }
            goto setPropDone;
        }

        /* Process options.logLevel */
        if ( !strcasecmp(name, IoTPConfig_options_logLevel)) {
            if (argptr == NULL || *argptr == '\0') {
                rc = IOTPRC_PARAM_NULL_VALUE;
            } else {
                if ( argptr && !strcasecmp(argptr, "error")) {
                    config->logLevel = LOGLEVEL_ERROR;
                } else if ( argptr && !strcasecmp(argptr, "warning")) {
                    config->logLevel = LOGLEVEL_WARN;
                } else if ( argptr && !strcasecmp(argptr, "warn")) {
                    config->logLevel = LOGLEVEL_WARN;
                } else if ( argptr && !strcasecmp(argptr, "info")) {
                    config->logLevel = LOGLEVEL_INFO;
                } else if ( argptr && !strcasecmp(argptr, "information")) {
                    config->logLevel = LOGLEVEL_INFO;
                } else if ( argptr && !strcasecmp(argptr, "debug")) {
                    config->logLevel = LOGLEVEL_DEBUG;
                } else {
                    rc = IOTPRC_PARAM_INVALID_VALUE;
                }
            }
            goto setPropDone;
        }

        /* Process options.mqtt.port */
        if ( !strcasecmp(name, IoTPConfig_options_mqtt_port)) {
            if (argint == 1883 || argint == 443 || argint == 8883) {
                config->mqttopts->port =  argint;
            } else if ((argptr != NULL && *argptr == '0') || (!argptr && argint == 0)) {
                config->mqttopts->port = 8883;
            } else {
                rc = IOTPRC_PARAM_INVALID_VALUE;
            }
            goto setPropDone;
        }

        /* Process options.mqtt.traceLevel */
        if ( !strcasecmp(name, IoTPConfig_options_mqtt_traceLevel)) {
            if (argint >= 1 && argint <= 7) {
                config->mqttopts->traceLevel =  argint;
            } else {
                rc = IOTPRC_PARAM_INVALID_VALUE;
            }
            goto setPropDone;
        }

        /* Process options.mqtt.transport */
        if ( !strcasecmp(name, IoTPConfig_options_mqtt_transport)) {
            if (argptr == NULL || *argptr == '\0') {
                rc = IOTPRC_PARAM_NULL_VALUE;
            } else {
                if ( argptr && ( !strcmp(argptr, "tcp") || !strcmp(argptr, "wss"))) {
                    if (config->mqttopts->transport)
                        iotp_utils_freePtr((void *)config->mqttopts->transport);
                    config->mqttopts->transport = strdup(argptr);
                } else {
                    rc = IOTPRC_PARAM_INVALID_VALUE;
                }
            }
            goto setPropDone;
        }

        /* Process options.mqtt.caFile */
        if ( !strcasecmp(name, IoTPConfig_options_mqtt_caFile)) {
            if (argint != 0) {
                rc = IOTPRC_PARAM_INVALID_VALUE;
                LOG(ERROR, "Invalid configuration value is specified: rc=%d", rc);
            } else if (argptr == NULL || *argptr == '\0') {
                if ( config->mqttopts->caFile ) 
                    iotp_utils_freePtr((void *)config->mqttopts->caFile);
                config->mqttopts->caFile = strdup("./IoTPlatform.pem");
            } else {
                if ( config->mqttopts->caFile ) 
                    iotp_utils_freePtr((void *)config->mqttopts->caFile);
                config->mqttopts->caFile = strdup(argptr);
            }
            goto setPropDone;
        }

        /* Process options.mqtt.cleanSession */
        if ( !strcasecmp(name, IoTPConfig_options_mqtt_cleanSession)) {
            if (argptr && (*argptr == '0' || *argptr == '1')) {
                config->mqttopts->cleanSession = argint;
            } else if (argptr && !strcmp(argptr,"true")) {
                config->mqttopts->cleanSession = 1; 
            } else if (argptr && !strcmp(argptr,"false")) {
                config->mqttopts->cleanSession = 0; 
            } else {
                rc = IOTPRC_PARAM_INVALID_VALUE;
            }
            goto setPropDone;
        }

        /* Process options.mqtt.cleanStart */
        if ( !strcasecmp(name, IoTPConfig_options_mqtt_cleanStart)) {
            if (argptr && (*argptr == '0' || *argptr == '1')) {
                config->mqttopts->cleanStart = argint;
            } else if (argptr && !strcmp(argptr,"true")) {
                config->mqttopts->cleanStart = 1;
            } else if (argptr && !strcmp(argptr,"false")) {
                config->mqttopts->cleanStart = 0;
            } else {
                rc = IOTPRC_PARAM_INVALID_VALUE;
            }
            goto setPropDone;
        }

        /* Process options.mqtt.sessionExpiry */
        if ( !strcasecmp(name, IoTPConfig_options_mqtt_sessionExpiry)) {
            if (argint >= 0 && argint <= 3600) {
                config->mqttopts->sessionExpiry = argint;
            } else {
                rc = IOTPRC_PARAM_INVALID_VALUE;
            }
            goto setPropDone;
        }

        /* Process options.mqtt.keepalive */
        if ( !strcasecmp(name, IoTPConfig_options_mqtt_keepalive)) {
            if (argint >= 0 || argint <= 720000) {
                config->mqttopts->keepalive =  argint;
            } else {
                rc = IOTPRC_PARAM_INVALID_VALUE;
            }
            goto setPropDone;
        }

        /* Process options.mqtt.sharedSubscription */
        if ( !strcasecmp(name, IoTPConfig_options_mqtt_sharedSubscription)) {
            if (argptr && (*argptr == '0' || *argptr == '1')) {
                config->mqttopts->sharedSubscription = argint;
            } else if (argptr && !strcmp(argptr,"true")) {
                config->mqttopts->sharedSubscription = 1;
            } else if (argptr && !strcmp(argptr,"false")) {
                config->mqttopts->sharedSubscription = 0;
            } else {
                rc = IOTPRC_PARAM_INVALID_VALUE;
            }
            goto setPropDone;
        }

        /* Process options.mqtt.validateServerCert */
        if ( !strcasecmp(name, IoTPConfig_options_mqtt_validateServerCert)) {
            if (argptr && (*argptr == '0' || *argptr == '1')) {
                config->mqttopts->validateServerCert = argint;
            } else if (argptr && !strcmp(argptr,"true")) {
                config->mqttopts->validateServerCert = 1;
            } else if (argptr && !strcmp(argptr,"false")) {
                config->mqttopts->validateServerCert = 0;
            } else {
                rc = IOTPRC_PARAM_INVALID_VALUE;
            }
            goto setPropDone;
        }

#ifdef HTTP_IMPLEMENTED
        /* Process options.http.validateServerCert */
        if ( !strcasecmp(name, IoTPConfig_options_http_validateServerCert)) {
            if (argptr && (*argptr == '0' || *argptr == '1')) {
                config->httpopts->validateServerCert = argint;
            } else if (argptr && !strcmp(argptr,"true")) {
                config->httpopts->validateServerCert = 1;
            } else if (argptr && !strcmp(argptr,"false")) {
                config->httpopts->validateServerCert = 0;
            } else {
                rc = IOTPRC_PARAM_INVALID_VALUE;
            }
            goto setPropDone;
        }

        /* Process options.http.caFile */
        if ( !strcasecmp(name, IoTPConfig_options_http_caFile)) {
            if (argint != 0) {
                rc = IOTPRC_PARAM_INVALID_VALUE;
                LOG(ERROR, "Invalid configuration value is specified: rc=%d", rc);
            } else if (argptr == NULL || *argptr == '\0') {
                if ( config->httpopts->caFile ) 
                    iotp_utils_freePtr((void *)config->httpopts->caFile);
                config->httpopts->caFile = strdup("./IoTPlatform.pem");
            } else {
                if ( config->httpopts->caFile ) 
                    iotp_utils_freePtr((void *)config->httpopts->caFile);
                config->httpopts->caFile = strdup(argptr);
            }
            goto setPropDone;
        }
#endif

        /* Process internal optional config item options.authMethod */
        if ( !strcasecmp(name, IoTPInternal_options_authMethod)) {
            if (argint != 0 ) {
                rc = IOTPRC_PARAM_INVALID_VALUE;
            } else if (argptr == NULL || *argptr == '\0') {
                config->authMethod =  0;
            } else if ( argptr && !strcasecmp(argptr, "token")) {
                config->authMethod =  1;
            } else if ( argptr && !strcmp(argptr, "cert")) {
                config->authMethod =  2;
            } else {
                rc = IOTPRC_PARAM_INVALID_VALUE;
            }
            goto setPropDone;
        }
    
        /* Process internal optional config item options.automaticReconnect */
        if ( !strcasecmp(name, IoTPInternal_options_automaticReconnect)) {
            if (argptr && (*argptr == '0' || *argptr == '1')) {
                config->automaticReconnect = argint;
            } else if (argptr && !strcmp(argptr,"true")) {
                config->automaticReconnect = 1;
            } else if (argptr && !strcmp(argptr,"false")) {
                config->automaticReconnect = 0;
            } else {
                rc = IOTPRC_PARAM_INVALID_VALUE;
            }
            goto setPropDone;
        }
    
    }

    /* Could not find any valid configuration */
    rc = IOTPRC_INVALID_PARAM;

setPropDone:

    if (rc != IOTPRC_SUCCESS) {
        LOG(ERROR, "Invalid configuration item (%s) or value (%s) is specified. rc=%d", 
            name?name:"", value?value:"",  rc);
    } else {
        LOG(INFO, "Set config item (%s) to (%s).", name?name:"", value?value:"");
    }

    return rc;
}

/* Function to set Watson IoT configuration object properties */
/* Reads configuration properties from file */
IOTPRC IoTPConfig_readConfigFile(IoTPConfig *config, const char *configFileName) 
{
    IOTPRC rc = IOTPRC_SUCCESS;

    char *line = NULL;
    char *propname = NULL;
    int proplen = 0;
    char *levelName[MAX_YAML_CONFIG_SECTIONS];
    FILE *fd;
    int i = 0;
    int setLevel = 0;
    int lastSpaces = 0;
    int indent = 0;

    /* sanity check */
    if (!config) {
        rc = IOTPRC_INVALID_HANDLE;
        LOG(ERROR, "Invalid config handle: rc=%d", rc);
        return rc;
    }
    if (!configFileName || *configFileName == '\0') {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(ERROR, "Invalid config file: %s", configFileName?configFileName:"");
        return rc;
    }
    if ((fd = fopen(configFileName, "r")) == NULL) {
        rc = IOTPRC_FILE_OPEN;
        LOG(ERROR, "Unable to open config file: %s", configFileName?configFileName:"");
        return rc;
    }

    LOG(DEBUG, "Read configuration from config file: %s", configFileName );

    /* init variables */
    line = (char *)malloc(512);
    for (i=0; i<MAX_YAML_CONFIG_SECTIONS; i++) levelName[i] = NULL;

    /* Process configuration file entries */
    while (fgets(line, 512, fd) != NULL) {
        char *prop = NULL;
        char *value = NULL;
        char *more = NULL;
        int nSpaces = 0;

        /* check number of leading white spaces */
        while (line[nSpaces] == ' ') nSpaces += 1;
        if ( nSpaces > lastSpaces ) {
            indent += 1;
            lastSpaces = nSpaces;
        } else if ( nSpaces < lastSpaces ) {
            indent -= 1;
            lastSpaces = nSpaces;
        }

        prop = iotp_utils_getToken(line, " \t\r\n", ":\r\n", &more);
        if (prop && prop[0] != '#') {
            char * cp = prop+strlen(prop); /* trim trailing white space */
            while (cp>prop && (cp[-1]==' ' || cp[-1]=='\t' ))
                cp--;
            *cp = 0;

            iotp_utils_trim(prop);

            /* get value */
            value = iotp_utils_getToken(more, " :\t\r\n", "\r\n", &more);
            if (!value) {
                value = "";
                setLevel = 1;
            }

            if (!prop || *prop == '\0') {
                rc = IOTPRC_INVALID_PARAM;
                LOG(WARN, "Invalid entry in config file: %s", line);
                break;
            }

            if ( setLevel ) {
                if ( levelName[indent] != NULL ) iotp_utils_freePtr((void *)levelName[indent]);
                levelName[indent] = strdup(prop);
            }
            if ( value && *value != '\0' ) {
                proplen = 0;
                for ( i=0; i<indent; i++) proplen += strlen(levelName[i]);
                proplen += strlen(prop) + indent + 1;
                propname = (char *)calloc(1, proplen);

                for ( i=0; i<indent; i++) {
                    strcat(propname, levelName[i]);
                    strcat(propname, ".");
                }

                strcat(propname, prop);

                LOG(DEBUG, "Process config parameter: %s  value=%s", propname, value);
                rc = IoTPConfig_setProperty(config, propname, value);
                iotp_utils_freePtr((void *)propname);
                if ( rc != IOTPRC_SUCCESS )
                    return rc;
            }
            setLevel = 0;

        }

        memset(line, 0, 512);
    }

    iotp_utils_freePtr((void *)line);
    for (i=0; i<MAX_YAML_CONFIG_SECTIONS; i++) {
        if ( levelName[i] != NULL ) iotp_utils_freePtr((void *)levelName[i]);
    }

    /* close configuration file */
    fclose(fd);

    return rc;
}


/* Reads configuration properties from environment variables */
IOTPRC IoTPConfig_readEnvironment(IoTPConfig *config) 
{
    IOTPRC rc = IOTPRC_SUCCESS;
    char *env = *environ;
    int index = 1;
    int totalProcessed = 0;
    int wiotpConfigFound = 0;
    int wiotpConfigValid = 0;

    /* sanity check */
    if (!config) {
        rc = IOTPRC_INVALID_HANDLE;
        LOG(ERROR, "Invalid config handle: rc=%d", rc);
        return rc;
    }

    for ( ; env; index++ )
    {
        char *envval = strdup(env);
        char *prop = strtok(envval, "=");
        char *value = strtok(NULL, "=");
        totalProcessed += 1;

        if ( prop && *prop != '\0' && !strncasecmp(prop, "WIOTP_", 6) && value && *value != '\0' )
        {
            IOTPRC rc1 = IOTPRC_SUCCESS;
            char *name = NULL;
            char *p = NULL;

            wiotpConfigFound += 1;

            /* replace _ with . */
            p = prop;
            for (; *p; ++p) {
                if (*p == '_') *p = '.';
            }

            /* set name */
            name = prop + 6;
 
            LOG(DEBUG, "Set parameter (%s) to (%s) from environment variable", name?name:"", value?value:"");
            rc1 = IoTPConfig_setProperty(config, name, value);
            /* Ignore invalid environment - just log errors */
            if ( rc1 != IOTPRC_SUCCESS ) {
                LOG(WARN, "Either environment variable (%s) could not be mapped to any configuration item or specified value is not valid", prop, value?value:"" );
            } else {
                wiotpConfigValid += 1;
            }
        }

        iotp_utils_freePtr((void *)envval);

        env = *(environ + index);
    }

    LOG(INFO, "Processed %d environment variables: WIOTP config items found=%d valid=%d", totalProcessed, wiotpConfigFound, wiotpConfigValid);

    return rc;
}


/* IoTPConfig_getProperty: Get IoTP configuration object properties */
IOTPRC IoTPConfig_getProperty(IoTPConfig *config, const char * name, char ** value, int len)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    if (!config) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(ERROR, "NULL configuration object is specified: rc=%d", rc);
        return rc;
    }
    if (!name || *name == '\0') {
        rc = IOTPRC_INVALID_PARAM;
        LOG(ERROR, "Invalid configuration parameter is specified: rc=%d", rc);
        return rc;
    }

    /* Process Identity configuration items */
    if ( !strncasecmp(name, "identity.", 9)) {
        /* Process identity.orgid */
        if ( !strcasecmp(name, IoTPConfig_identity_orgId)) {
            snprintf(*value, len, "%s", config->identity->orgId? config->identity->orgId : "");
            goto getPropDone;
        }
    
        /* Process identity.typeId */
        if ( !strcasecmp(name, IoTPConfig_identity_typeId)) {
            snprintf(*value, len, "%s", config->identity->typeId? config->identity->typeId : "");
            goto getPropDone;
        }

        /* Process identity.deviceId */
        if ( !strcasecmp(name, IoTPConfig_identity_deviceId)) {
            snprintf(*value, len, "%s", config->identity->deviceId? config->identity->deviceId : "");
            goto getPropDone;
        }
    
        /* Process identity.appId */
        if ( !strcasecmp(name, IoTPConfig_identity_appId)) {
            snprintf(*value, len, "%s", config->identity->appId? config->identity->appId : "");
            goto getPropDone;
        }
    }
    
    /* Process Auth configuration items */
    if ( !strncasecmp(name, "auth.", 5)) {
        /* Process auth.keyStore */
        if ( !strcasecmp(name, IoTPConfig_auth_keyStore)) {
            snprintf(*value, len, "%s", config->auth->keyStore? config->auth->keyStore : "");
            goto getPropDone;
        }

        /* Process auth.privateKey */
        if ( !strcasecmp(name, IoTPConfig_auth_privateKey)) {
            snprintf(*value, len, "%s", config->auth->privateKey? config->auth->privateKey : "");
            goto getPropDone;
        }

        /* Process auth.privateKeyPassword */
        if ( !strcasecmp(name, IoTPConfig_auth_privateKeyPassword)) {
            snprintf(*value, len, "%s", config->auth->privateKeyPassword? config->auth->privateKeyPassword : "");
            goto getPropDone;
        }

        /* Process auth.token */
        if ( !strcasecmp(name, IoTPConfig_auth_token)) {
            snprintf(*value, len, "%s", config->auth->token? config->auth->token : "");
            goto getPropDone;
        }

        /* Process auth.APIKey */
        if ( !strcasecmp(name, IoTPConfig_auth_APIKey)) {
            snprintf(*value, len, "%s", config->auth->apiKey? config->auth->apiKey : "");
            goto getPropDone;
        }
    }


    /* Process Optional configuration items */
    if ( !strncasecmp(name, "Options.", 8)) {
        /* Process options.domain */
        if ( !strcasecmp(name, IoTPConfig_options_domain)) {
            snprintf(*value, len, "%s", config->domain? config->domain : "");
            goto getPropDone;
        }

        /* Process options.logLevel */
        if ( !strcasecmp(name, IoTPConfig_options_logLevel)) {
            if ( config->logLevel == LOGLEVEL_ERROR ) {
                snprintf(*value, len, "error");
            } else if ( config->logLevel == LOGLEVEL_WARN ) {
                snprintf(*value, len, "warning");
            } else if ( config->logLevel == LOGLEVEL_INFO ) {
                snprintf(*value, len, "info");
            } else if ( config->logLevel == LOGLEVEL_DEBUG ) {
                snprintf(*value, len, "debug");
            }
            goto getPropDone;
        }

        /* Process options.mqtt.port */
        if ( !strcasecmp(name, IoTPConfig_options_mqtt_port)) {
            snprintf(*value, len, "%d", config->mqttopts->port);
            goto getPropDone;
        }

        /* Process options.mqtt.traceLevel */
        if ( !strcasecmp(name, IoTPConfig_options_mqtt_traceLevel)) {
            snprintf(*value, len, "%d", config->mqttopts->traceLevel);
            goto getPropDone;
        }

        /* Process options.mqtt.transport */
        if ( !strcasecmp(name, IoTPConfig_options_mqtt_transport)) {
            snprintf(*value, len, "%s", config->mqttopts->transport);
            goto getPropDone;
        }

        /* Process options.mqtt.caFile */
        if ( !strcasecmp(name, IoTPConfig_options_mqtt_caFile)) {
            snprintf(*value, len, "%s", config->mqttopts->caFile);
            goto getPropDone;
        }

        /* Process options.mqtt.cleanSession */
        if ( !strcasecmp(name, IoTPConfig_options_mqtt_cleanSession)) {
            if (config->mqttopts->cleanSession == 0) {
                snprintf(*value, len, "false");
            } else if (config->mqttopts->cleanSession == 1) {
                snprintf(*value, len, "true");
            }
            goto getPropDone;
        }

        /* Process options.mqtt.cleanStart */
        if ( !strcasecmp(name, IoTPConfig_options_mqtt_cleanStart)) {
            if (config->mqttopts->cleanStart == 0) {
                snprintf(*value, len, "false");
            } else if (config->mqttopts->cleanStart == 1) {
                snprintf(*value, len, "true");
            }
            goto getPropDone;
        }

        /* Process options.mqtt.sessionExpiry */
        if ( !strcasecmp(name, IoTPConfig_options_mqtt_sessionExpiry)) {
            snprintf(*value, len, "%d", config->mqttopts->sessionExpiry);
            goto getPropDone;
        }

        /* Process options.mqtt.keepalive */
        if ( !strcasecmp(name, IoTPConfig_options_mqtt_keepalive)) {
            snprintf(*value, len, "%d", config->mqttopts->keepalive);
            goto getPropDone;
        }

        /* Process options.mqtt.sharedSubscription */
        if ( !strcasecmp(name, IoTPConfig_options_mqtt_sharedSubscription)) {
            if (config->mqttopts->sharedSubscription == 0) {
                snprintf(*value, len, "false");
            } else if (config->mqttopts->sharedSubscription == 1) {
                snprintf(*value, len, "true");
            }
            goto getPropDone;
        }

        /* Process options.mqtt.validateServerCert */
        if ( !strcasecmp(name, IoTPConfig_options_mqtt_validateServerCert)) {
            if (config->mqttopts->validateServerCert == 0) {
                snprintf(*value, len, "false");
            } else if (config->mqttopts->validateServerCert == 1) {
                snprintf(*value, len, "true");
            }
            goto getPropDone;
        }

#ifdef HTTP_IMPLEMENTED
        /* Process options.http.validateServerCert */
        if ( !strcasecmp(name, IoTPConfig_options_http_validateServerCert)) {
            if (config->httpopts->validateServerCert == 0) {
                snprintf(*value, len, "false");
            } else if (config->httpopts->validateServerCert == 1) {
                snprintf(*value, len, "true");
            }
            goto getPropDone;
        }

        /* Process options.http.caFile */
        if ( !strcasecmp(name, IoTPConfig_options_http_caFile)) {
            snprintf(*value, len, "%s", config->httpopts->caFile);
            goto getPropDone;
        }
#endif

        /* Process internal options.authMethod */
        if ( !strcasecmp(name, IoTPInternal_options_authMethod)) {
            if (config->authMethod == 0) {
                snprintf(*value, len, "");
            } else if (config->authMethod == 1) {
                snprintf(*value, len, "token");
            } else if (config->authMethod == 2) {
                snprintf(*value, len, "cert");
            }
            goto getPropDone;
        }
    
        /* Process internal options.automaticReconnect */
        if ( !strcasecmp(name, IoTPInternal_options_automaticReconnect)) {
            if (config->automaticReconnect == 0) {
                snprintf(*value, len, "false");
            } else if (config->automaticReconnect == 1) {
                snprintf(*value, len, "true");
            }
            goto getPropDone;
        }
    
    }

    /* Could not find any valid configuration */
    rc = IOTPRC_INVALID_PARAM;

getPropDone:

    if (rc != IOTPRC_SUCCESS) {
        LOG(ERROR, "Invalid configuration item (%s) is specified. rc=%d", name?name:"",  rc);
    } else {
        LOG(DEBUG, "Get config item (%s) to (%s).", name?name:"", *value? *value:"");
    }

    return rc;
}


