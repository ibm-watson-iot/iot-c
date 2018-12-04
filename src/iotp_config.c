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
IoTP_RC IoTPConfig_setLogHandler(IoTPLogTypes type, void * handler)
{
    IoTP_RC rc = IoTP_SUCCESS;
    rc = iotp_utils_setLogHandler(type, handler);
    return rc;
}

/* IoTPConfig_create: Creates IBM Watson IoT client configuration object */
IoTP_RC IoTPConfig_create(IoTPConfig **config, const char * configFileName)
{
    IoTP_RC rc = IoTP_SUCCESS;

    /* Check if config handle is valid i.e. not NULL or already inited */
    if ( config == NULL ) {
        rc = IoTP_RC_PARAM_NULL_VALUE;
        LOG(ERROR, "NULL configuration object is specified: rc=%d", rc);
        return rc;
    }
    if ( config && *config != NULL ) {
        rc = IoTP_RC_INVALID_HANDLE;
        LOG(ERROR, "Configuration object is already initialized: rc=%d", rc);
        return rc;
    }


    iotp_utils_writeClientVersion();
    LOG(INFO, "Create config object from file: %s", configFileName?configFileName:"");

    /* Create configuration handle */
    *config = (IoTPConfig *)calloc(1, sizeof(IoTPConfig));
    if ( *config == NULL ) {
        rc = IoTP_RC_NOMEM;
        return rc;
    }

    /* Set defaults */
    (*config)->domain = strdup("internetofthings.ibmcloud.com");
    (*config)->port = 8883;
    (*config)->serverCertificatePath = strdup("./IoTPlatform.pem");
    (*config)->logLevel = LOGLEVEL_TRACE;
    (*config)->MQTTTraceLevel = 5;
    (*config)->device = (iotc *)calloc(1, sizeof(iotc));
    (*config)->gateway = (iotc *)calloc(1, sizeof(iotc));
    (*config)->application = (iota *)calloc(1, sizeof(iota));
    (*config)->keepAliveInterval = 60;
    (*config)->automaticReconnect = 0;

    /* If configuration file is specified - process it */
    if ( configFileName && *configFileName != '\0' ) {
        rc = IoTPConfig_readConfigFile(*config, configFileName);
    }

    return rc;
}


/* Clears all properties from a configuration object */
IoTP_RC IoTPConfig_clear(IoTPConfig *config) 
{
    IoTP_RC rc = IoTP_SUCCESS;

    /* sanity check */
    if (!config) {
        rc = IoTP_RC_PARAM_NULL_VALUE;
        LOG(ERROR, "NULL configuration object is specified: rc=%d", rc);
    } else {
        /* free config object */
        iotp_utils_freePtr((void *)config->orgId);
        iotp_utils_freePtr((void *)config->domain);
        iotp_utils_freePtr((void *)config->serverCertificatePath);
        if ( config->device != NULL ) {
            iotc *device = config->device;
            iotp_utils_freePtr((void *)device->typeId);
            iotp_utils_freePtr((void *)device->deviceId);
            iotp_utils_freePtr((void *)device->authToken);
            iotp_utils_freePtr((void *)device->certificatePath);
            iotp_utils_freePtr((void *)device->keyPath);
            iotp_utils_freePtr((void *)config->device);
        }
        if ( config->gateway != NULL ) {
            iotc *gateway = config->gateway;
            iotp_utils_freePtr((void *)gateway->typeId);
            iotp_utils_freePtr((void *)gateway->deviceId);
            iotp_utils_freePtr((void *)gateway->authToken);
            iotp_utils_freePtr((void *)gateway->certificatePath);
            iotp_utils_freePtr((void *)gateway->keyPath);
            iotp_utils_freePtr((void *)config->gateway);
        }
        if ( config->application != NULL ) {
            iota *application = config->application;
            iotp_utils_freePtr((void *)application->appId);
            iotp_utils_freePtr((void *)application->APIKey);
            iotp_utils_freePtr((void *)application->authToken);
            iotp_utils_freePtr((void *)config->application);
        }
        iotp_utils_freePtr(config);
    }
    return rc;
}


/* IoTPConfig_setProperty: Set IoTP configuration object properties */
IoTP_RC IoTPConfig_setProperty(IoTPConfig *config, const char * name, const char * value)
{
    IoTP_RC rc = IoTP_SUCCESS;

    char *argptr = NULL;
    int   argint = 0;

    /* sanity check */
    if (!config) {
        rc = IoTP_RC_PARAM_NULL_VALUE;
        LOG(ERROR, "NULL configuration object is specified: rc=%d", rc);
        return rc;
    }
    if (!name || *name == '\0') {
        rc = IoTP_RC_INVALID_PARAM;
        LOG(ERROR, "Invalid configuration parameter is specified: rc=%d", rc);
        return rc;
    }

    argptr = (char *)value;
    if ( argptr && *argptr != '\0' ) {
        argint = atoi(argptr);
    }

    /* Process Platform configuration items */
    if ( !strncasecmp(name, "Platform.", 9)) {
        /* Process Platform.domain */
        if ( !strcasecmp(name, IoTPConfig_Platform_domain)) {
            if (argint != 0) {
                rc = IoTP_RC_PARAM_INVALID_VALUE;
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
            if (rc == IoTP_SUCCESS) {
                LOG(INFO, "Set config parameter: %s To: %s", IoTPConfig_Platform_domain, config->domain);
            }
            goto setPropDone;
        }
    
        /* Process Platform.port */
        if ( !strcasecmp(name, IoTPConfig_Platform_port)) {
            if (argint == 1883 || argint == 443 || argint == 8883) {
                config->port =  argint;
            } else if ((argptr != NULL && *argptr == '0') || (!argptr && argint == 0)) {
                config->port = 8883;
            } else {
                rc = IoTP_RC_PARAM_INVALID_VALUE;
                LOG(ERROR, "Invalid configuration value is specified: rc=%d", rc);
            }
            if (rc == IoTP_SUCCESS) {
                LOG(INFO, "Set config parameter: %s To: %d", IoTPConfig_Platform_port, config->port);
            }
            goto setPropDone;
        }

        /* Process Platform.serverCertificatePath */
        if ( !strcasecmp(name, IoTPConfig_Platform_serverCertificatePath)) {
            if (argint != 0) {
                rc = IoTP_RC_PARAM_INVALID_VALUE;
                LOG(ERROR, "Invalid configuration value is specified: rc=%d", rc);
            } else if (argptr == NULL || *argptr == '\0') {
                if ( config->serverCertificatePath ) 
                    iotp_utils_freePtr((void *)config->serverCertificatePath);
                config->serverCertificatePath = strdup("./IoTFoundation.pem");
            } else {
                if ( config->serverCertificatePath ) 
                    iotp_utils_freePtr((void *)config->serverCertificatePath);
                config->serverCertificatePath = strdup(argptr);
            }
            if (rc == IoTP_SUCCESS) {
                LOG(INFO, "Set config parameter: %s To: %d", IoTPConfig_Platform_serverCertificatePath, config->serverCertificatePath);
            }
            goto setPropDone;
        }
    }


    /* Process Organization configuration items */
    if ( !strncasecmp(name, "Organization.", 13)) {
        /* Process Organization.id */
        if ( !strcasecmp(name, IoTPConfig_Organization_id)) {
            if (argptr == NULL || *argptr == '\0') {
                rc = IoTP_RC_PARAM_NULL_VALUE;
                LOG(ERROR, "NULL configuration value is specified: rc=%d", rc);
            } else if (argint != 0 ) {
                rc = IoTP_RC_PARAM_INVALID_VALUE;
                LOG(ERROR, "Invalid configuration value is specified: rc=%d", rc);
            } else {
                int len = strlen(argptr);
                if ( len != 6 ) {
                    rc = IoTP_RC_PARAM_INVALID_VALUE;
                    LOG(ERROR, "Invalid configuration value is specified: rc=%d", rc);
                } else {
                    if ( config->orgId ) 
                        iotp_utils_freePtr((void *)config->orgId);
                    config->orgId = strdup(argptr);
                }
            }
            if (rc == IoTP_SUCCESS) {
                LOG(INFO, "Set config parameter: %s To: %s", IoTPConfig_Organization_id, config->orgId);
            }
            goto setPropDone;
        }
    }


    /* Process Device configuration items */
    if ( !strncasecmp(name, "Device.", 7)) {
        /* Process Device.typeId */
        if ( !strcasecmp(name, IoTPConfig_Device_typeId)) {
            if (argptr == NULL || (argptr && *argptr == '\0')) {
                rc = IoTP_RC_PARAM_NULL_VALUE;
                LOG(ERROR, "NULL configuration value is specified: rc=%d", rc);
            } else {
                if ( config->device->typeId ) 
                    iotp_utils_freePtr((void *)config->device->typeId);
                config->device->typeId =  strdup(argptr);
            }
            if (rc == IoTP_SUCCESS) {
                LOG(INFO, "Set config parameter: %s To: %s", IoTPConfig_Device_typeId, config->device->typeId);
            }
            goto setPropDone;
        }

        /* Process Device.deviceId */
        if ( !strcasecmp(name, IoTPConfig_Device_deviceId)) {
            if (argptr == NULL || (argptr && *argptr == '\0')) {
                LOG(ERROR, "NULL configuration value is specified: rc=%d", rc);
                rc = IoTP_RC_PARAM_NULL_VALUE;
            } else {
                if ( config->device->deviceId ) 
                    iotp_utils_freePtr((void *)config->device->deviceId);
                config->device->deviceId =  strdup(argptr);
            }
            if (rc == IoTP_SUCCESS) {
                LOG(INFO, "Set config parameter: %s To: %s", IoTPConfig_Device_deviceId, config->device->deviceId);
            }
            goto setPropDone;
        }
    
        /* Process Device.authMethod */
        if ( !strcasecmp(name, IoTPConfig_Device_authMethod)) {
            if (argint != 0 ) {
                rc = IoTP_RC_PARAM_INVALID_VALUE;
                LOG(ERROR, "Invalid configuration value is specified: rc=%d", rc);
            } else if (argptr == NULL || *argptr == '\0') {
                config->device->authMethod =  0;
            } else if ( argptr && !strcasecmp(argptr, "token")) {
                config->device->authMethod =  1;
            } else if ( argptr && !strcmp(argptr, "cert")) {
                config->device->authMethod =  2;
            } else {
                rc = IoTP_RC_PARAM_INVALID_VALUE;
                LOG(ERROR, "Invalid configuration value is specified: rc=%d", rc);
            }
            if (rc == IoTP_SUCCESS) {
                LOG(INFO, "Set config parameter: %s To: %d", IoTPConfig_Device_authMethod, config->device->authMethod);
            }
            goto setPropDone;
        }
    
        /* Process Device.authToken */
        if ( !strcasecmp(name, IoTPConfig_Device_authToken)) {
            if (argint != 0 ) {
                rc = IoTP_RC_PARAM_INVALID_VALUE;
                LOG(ERROR, "Invalid configuration value is specified: rc=%d", rc);
            } else if ( !argptr || *argptr == '\0' ) {
                if ( config->device->authToken ) 
                    iotp_utils_freePtr((void *)config->device->authToken);
                config->device->authToken =  NULL;
            } else {
                if ( config->device->authToken ) 
                    iotp_utils_freePtr((void *)config->device->authToken);
                config->device->authToken =  strdup(argptr);
            }
            if (rc == IoTP_SUCCESS) {
                LOG(INFO, "Set config parameter: %s To: %s", IoTPConfig_Device_authToken, config->device->authToken? config->device->authToken : "NULL");
            }
            goto setPropDone;
        }
    
        /* Process Device.certificatePath */
        if ( !strcasecmp(name, IoTPConfig_Device_certificatePath)) {
            if (argint != 0 ) {
                rc = IoTP_RC_PARAM_INVALID_VALUE;
                LOG(ERROR, "Invalid configuration value is specified: rc=%d", rc);
            } else if ( !argptr || *argptr == '\0' ) {
                if ( config->device->certificatePath ) 
                    iotp_utils_freePtr((void *)config->device->certificatePath);
                config->device->certificatePath =  NULL;
            } else {
                if ( config->device->certificatePath ) 
                    iotp_utils_freePtr((void *)config->device->certificatePath);
                config->device->certificatePath =  strdup(argptr);
            }
            if (rc == IoTP_SUCCESS) {
                LOG(INFO, "Set config parameter: %s To: %s", IoTPConfig_Device_certificatePath, config->device->certificatePath? config->device->certificatePath : "NULL");
            }
            goto setPropDone;
        }
    
        /* Process Device.keyPath */
        if ( !strcasecmp(name, IoTPConfig_Device_keyPath)) {
            if (argint != 0 ) {
                rc = IoTP_RC_PARAM_INVALID_VALUE;
                LOG(ERROR, "Invalid configuration value is specified: rc=%d", rc);
            } else if ( !argptr || *argptr == '\0' ) {
                if ( config->device->keyPath ) 
                    iotp_utils_freePtr((void *)config->device->keyPath);
                config->device->keyPath =  NULL;
            } else {
                if ( config->device->keyPath ) 
                    iotp_utils_freePtr((void *)config->device->keyPath);
                config->device->keyPath =  strdup(argptr);
            }
            if (rc == IoTP_SUCCESS) {
                LOG(INFO, "Set config parameter: %s To: %s", IoTPConfig_Device_keyPath, config->device->keyPath? config->device->keyPath : "NULL" );
            }
            goto setPropDone;
        }
    }


    /* Process Gateway configuration items */
    if ( !strncasecmp(name, "Gateway.", 8)) {
        /* Process Gateway.typeId */
        if ( !strcasecmp(name, IoTPConfig_Gateway_typeId)) {
            if (argptr == NULL || (argptr && *argptr == '\0')) {
                rc = IoTP_RC_PARAM_NULL_VALUE;
                LOG(ERROR, "NULL configuration value is specified: rc=%d", rc);
            } else {
                if ( config->gateway->typeId ) 
                    iotp_utils_freePtr((void *)config->gateway->typeId);
                config->gateway->typeId =  strdup(argptr);
            }
            if (rc == IoTP_SUCCESS) {
                LOG(INFO, "Set config parameter: %s To: %s", IoTPConfig_Gateway_typeId, config->gateway->typeId);
            }
            goto setPropDone;
        }

        /* Process Gateway.deviceId */
        if ( !strcasecmp(name, IoTPConfig_Gateway_deviceId)) {
            if (argptr == NULL || (argptr && *argptr == '\0')) {
                rc = IoTP_RC_PARAM_NULL_VALUE;
                LOG(ERROR, "NULL configuration value is specified: rc=%d", rc);
            } else {
                if ( config->gateway->deviceId ) 
                    iotp_utils_freePtr((void *)config->gateway->deviceId);
                config->gateway->deviceId =  strdup(argptr);
            }
            if (rc == IoTP_SUCCESS) {
                LOG(INFO, "Set config parameter: %s To: %s", IoTPConfig_Gateway_deviceId, config->gateway->deviceId);
            }
            goto setPropDone;
        }
    
        /* Process Gateway.authMethod */
        if ( !strcasecmp(name, IoTPConfig_Gateway_authMethod)) {
            if (argint != 0 ) {
                rc = IoTP_RC_PARAM_INVALID_VALUE;
                LOG(ERROR, "Invalid configuration value is specified: rc=%d", rc);
            } else if (argptr == NULL || (argptr && *argptr == '\0')) {
                config->gateway->authMethod =  0;
            } else if ( argptr && !strcasecmp(argptr, "token")) {
                config->gateway->authMethod =  1;
            } else if ( argptr && !strcmp(argptr, "cert")) {
                config->gateway->authMethod =  2;
            } else {
                rc = IoTP_RC_PARAM_INVALID_VALUE;
                LOG(ERROR, "Invalid configuration value is specified: rc=%d", rc);
            }
            if (rc == IoTP_SUCCESS) {
                LOG(INFO, "Set config parameter: %s To: %d", IoTPConfig_Gateway_authMethod, config->gateway->authMethod);
            }
            goto setPropDone;
        }
    
        /* Process Gateway.authToken */
        if ( !strcasecmp(name, IoTPConfig_Gateway_authToken)) {
            if (argint != 0 ) {
                rc = IoTP_RC_PARAM_INVALID_VALUE;
                LOG(ERROR, "Invalid configuration value is specified: rc=%d", rc);
            } else if ( !argptr || (argptr && *argptr == '\0')) {
                if ( config->gateway->authToken ) 
                    iotp_utils_freePtr((void *)config->gateway->authToken);
                config->gateway->authToken =  NULL;
            } else {
                if ( config->gateway->authToken ) 
                    iotp_utils_freePtr((void *)config->gateway->authToken);
                config->gateway->authToken =  strdup(argptr);
            }
            if (rc == IoTP_SUCCESS) {
                LOG(INFO, "Set config parameter: %s To: %s", IoTPConfig_Gateway_authToken, config->gateway->authToken? config->gateway->authToken : "NULL");
            }
            goto setPropDone;
        }
    
        /* Process Gateway.certificatePath */
        if ( !strcasecmp(name, IoTPConfig_Gateway_certificatePath)) {
            if (argint != 0 ) {
                rc = IoTP_RC_PARAM_INVALID_VALUE;
                LOG(ERROR, "Invalid configuration value is specified: rc=%d", rc);
            } else if ( !argptr || (argptr && *argptr == '\0')) {
                if ( config->gateway->certificatePath ) 
                    iotp_utils_freePtr((void *)config->gateway->certificatePath);
                config->gateway->certificatePath =  NULL;
            } else {
                if ( config->gateway->certificatePath ) 
                    iotp_utils_freePtr((void *)config->gateway->certificatePath);
                config->gateway->certificatePath =  strdup(argptr);
            }
            if (rc == IoTP_SUCCESS) {
                LOG(INFO, "Set config parameter: %s To: %s", IoTPConfig_Gateway_certificatePath, config->gateway->certificatePath? config->gateway->certificatePath : "NULL");
            }
            goto setPropDone;
        }
    
        /* Process Gateway.keyPath */
        if ( !strcasecmp(name, IoTPConfig_Gateway_keyPath)) {
            if (argint != 0 ) {
                rc = IoTP_RC_PARAM_INVALID_VALUE;
                LOG(ERROR, "Invalid configuration value is specified: rc=%d", rc);
            } else if ( !argptr || (argptr && *argptr == '\0')) {
                if ( config->gateway->keyPath ) 
                    iotp_utils_freePtr((void *)config->gateway->keyPath);
                config->gateway->keyPath =  NULL;
            } else {
                if ( config->gateway->keyPath ) 
                    iotp_utils_freePtr((void *)config->gateway->keyPath);
                config->gateway->keyPath =  strdup(argptr);
            }
            if (rc == IoTP_SUCCESS) {
                LOG(INFO, "Set config parameter: %s To: %s", IoTPConfig_Gateway_keyPath, config->gateway->keyPath? config->gateway->keyPath : "NULL" );
            }
            goto setPropDone;
        }
    }


    /* Process Application configuration items */
    if ( !strncasecmp(name, "Application.", 12)) {
        /* Process Application.appId */
        if ( !strcasecmp(name, IoTPConfig_Application_appId)) {
            if (argint != 0 ) {
                rc = IoTP_RC_PARAM_INVALID_VALUE;
                LOG(ERROR, "Invalid configuration value is specified: rc=%d", rc);
            } else if (argptr == NULL || (argptr && *argptr == '\0')) {
                rc = IoTP_RC_PARAM_NULL_VALUE;
                LOG(ERROR, "NULL configuration value is specified: rc=%d", rc);
            } else {
                if ( config->application->appId ) 
                    iotp_utils_freePtr((void *)config->application->appId);
                config->application->appId = strdup(argptr);
            }
            goto setPropDone;
        }

        /* Process Application.APIKey */
        if ( !strcasecmp(name, IoTPConfig_Application_APIKey)) {
            if (argint != 0 ) {
                rc = IoTP_RC_PARAM_INVALID_VALUE;
                LOG(ERROR, "Invalid configuration value is specified: rc=%d", rc);
            } else if (argptr == NULL || (argptr && *argptr == '\0')) {
                rc = IoTP_RC_PARAM_NULL_VALUE;
                LOG(ERROR, "NULL configuration value is specified: rc=%d", rc);
            } else {
                if ( config->application->APIKey ) 
                    iotp_utils_freePtr((void *)config->application->APIKey);
                config->application->APIKey = strdup(argptr);
            }
            goto setPropDone;
        }

        /* Process Application.authToken */
        if ( !strcasecmp(name, IoTPConfig_Application_authToken)) {
            if (argint != 0 ) {
                rc = IoTP_RC_PARAM_INVALID_VALUE;
                LOG(ERROR, "Invalid configuration value is specified: rc=%d", rc);
            } else if (argptr == NULL || (argptr && *argptr == '\0')) {
                rc = IoTP_RC_PARAM_NULL_VALUE;
                LOG(ERROR, "NULL configuration value is specified: rc=%d", rc);
            } else {
                if ( config->application->authToken ) 
                    iotp_utils_freePtr((void *)config->application->authToken);
                config->application->authToken = strdup(argptr);
            }
            goto setPropDone;
        }
    }


    /* Process Debug configuration items */
    if ( !strncasecmp(name, "Debug.", 6)) {
        /* Process Debug.logLevel */
        if ( !strcasecmp(name, IoTPConfig_Debug_logLevel)) {
            argint = atoi(argptr); 
            LOG(INFO, "argint=%d", argint);
            if (argint < 1 || argint > 5) {
                rc = IoTP_RC_PARAM_INVALID_VALUE;
                LOG(ERROR, "Invalid configuration value is specified: rc=%d", rc);
            } else {
                config->logLevel = argint;
                iotp_utils_setLogLevel(config->logLevel);
            }
            goto setPropDone;
        }

        /* Process Debug.MQTTTraceLevel */
        if ( !strcasecmp(name, IoTPConfig_Debug_MQTTTraceLevel)) {
            if (argint < 1 || argint > 6) {
                rc = IoTP_RC_PARAM_INVALID_VALUE;
                LOG(ERROR, "Invalid configuration value is specified: rc=%d", rc);
            } else {
                config->MQTTTraceLevel = argint;
            }
            goto setPropDone;
        }
    }

    /* Could not find any valid configuration */
    rc = IoTP_RC_INVALID_PARAM;
    LOG(ERROR, "Invalid configuration parameter is specified: rc=%d", rc);

setPropDone:

    if (rc != IoTP_SUCCESS) {
        LOG(ERROR, "Could not set configuration parameter %s. rc=%d", name, rc);
    }

    return rc;
}

/* Function to set Watson IoT configuration object properties */
/* Reads configuration properties from file */
IoTP_RC IoTPConfig_readConfigFile(IoTPConfig *config, const char *configFileName) 
{
    IoTP_RC rc = IoTP_SUCCESS;

    char line[256];
    char *propname = NULL;
    int linenum = 0;
    FILE *fd;

    /* sanity check */
    if (!config) {
        rc = IoTP_RC_INVALID_HANDLE;
        LOG(ERROR, "Invalid config handle: rc=%d", rc);
        return rc;
    }
    if (!configFileName || *configFileName == '\0') {
        rc = IoTP_RC_PARAM_NULL_VALUE;
        LOG(ERROR, "Invalid config file: %s", configFileName?configFileName:"");
        return rc;
    }
    if ((fd = fopen(configFileName, "r")) == NULL) {
        rc = IoTP_RC_FILE_OPEN;
        LOG(ERROR, "Unable to open config file: %s", configFileName?configFileName:"");
        return rc;
    }

    LOG(TRACE, "Process config file: %s", configFileName?configFileName:"" );

    /* Process configuration file entries */
    if (rc == IoTP_SUCCESS) {

        char *category = NULL;

        while (fgets(line, 256, fd) != NULL) {
            char *prop = NULL;
            char *value = NULL;
            int len = 0;

            linenum++;

            if (line[0] == '#' || line[0] == '\0')
                 continue;

            if (line[0] == ' ') {

                /* process category items */
                prop = strtok(line, ":");
                value = strtok(NULL, ":");
                if (prop) iotp_utils_trim(prop);
                if (value) iotp_utils_trim(value);

                /* ignore comment line */
                if (prop && *prop == '#')
                    continue;

                if (!prop || *prop == '\0' || !value || !category) {
                    rc = IoTP_RC_INVALID_PARAM;
                    LOG(WARN, "Invalid entry in config file: %s", line);
                    break;
                }

                /* process config item of the category */
                len = strlen(category) + strlen(prop) + 2;
                propname = (char *)malloc(len);
                snprintf(propname, len, "%s.%s", category, prop);
                LOG(DEBUG, "Process config parameter: %s  value=%s", propname, value);
                rc = IoTPConfig_setProperty(config, propname, value);
                iotp_utils_freePtr((void *)propname);
                if ( rc != IoTP_SUCCESS )
                    return rc;

            } else {

                /* process category name */
                prop = strtok(line, ":");
                value = strtok(NULL, ":");
                if (prop) iotp_utils_trim(prop);
                if (value) iotp_utils_trim(value);

                if (!prop || *prop == '\0' || (value && *value != '\0') ) {
                    continue;
                }

                /* set category value */
                if (category) iotp_utils_freePtr((void *)category);
                category = strdup(prop);

            }
        }

        if (category) iotp_utils_freePtr((void *)category);

        /* close configuration file */
        fclose(fd);
    }

    return rc;
}


/* Reads configuration properties from environment variables */
IoTP_RC IoTPConfig_readEnvironment(IoTPConfig *config) 
{
    IoTP_RC rc = IoTP_SUCCESS;
    char *env = *environ;
    int i = 1;

    /* sanity check */
    if (!config) {
        rc = IoTP_RC_INVALID_HANDLE;
        LOG(ERROR, "Invalid config handle: rc=%d", rc);
        return rc;
    }

    for ( ; env; i++ )
    {
        char *envval = strdup(env);
        char *prop = strtok(envval, "=");
        char *value = strtok(NULL, "=");
        if (prop) iotp_utils_trim(prop);
        if (value) iotp_utils_trim(value);

        if ( prop && !strncasecmp(prop, "IoTPConfig.", 11) && value && *value != '\0' )
        {
            char *name = prop + 11;
            rc = IoTPConfig_setProperty(config, name, value);
            if ( rc != IoTP_SUCCESS )
                return rc;
        }

        env = *(environ + i);
    }

    return rc;
}


/* IoTPConfig_getProperty: Get IoTP configuration object properties */
IoTP_RC IoTPConfig_getProperty(IoTPConfig *config, const char * name, char ** value, int len)
{
    IoTP_RC rc = IoTP_SUCCESS;

    if (!config) {
        rc = IoTP_RC_INVALID_HANDLE;
        LOG(ERROR, "Invalid config handle: rc=%d", rc);
        return rc;
    }
    if (!name || *name == '\0') {
        rc = IoTP_RC_INVALID_PARAM;
        return rc;
    }

    /* Process Platform configuration items */
    if ( !strncasecmp(name, "Platform.", 9)) {
        if ( !strcasecmp(name, IoTPConfig_Platform_domain)) {
            snprintf(*value, len, "%s", config->domain);
            goto getPropDone;
        }
        if ( !strcasecmp(name, IoTPConfig_Platform_port)) {
            snprintf(*value, len, "%d", config->port);
            goto getPropDone;
        }
        if ( !strcasecmp(name, IoTPConfig_Platform_serverCertificatePath)) {
            snprintf(*value, len, "%s", config->serverCertificatePath);
            goto getPropDone;
        }
    }


    /* Process Organization configuration items */
    if ( !strncasecmp(name, "Organization.", 13)) {
        if ( !strcasecmp(name, IoTPConfig_Organization_id)) {
            snprintf(*value, len, "%s", config->orgId);
            goto getPropDone;
        }
    }


    rc = IoTP_RC_INVALID_PARAM;

getPropDone:

    return rc;
}

