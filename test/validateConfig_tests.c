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

#include "test_utils.h"
#include "iotp_config.h"

/*
 * validateConfig_tests.c: IBM Watson IoT Platform C Client Configuration API validation tests
 *
 * This file contains test functions to test validation of following APIs:
 * - IoTPConfig_setHandler
 * - IoTPConfig_create
 * - IoTPConfig_readConfigFile
 * - IoTPConfig_readEnvironment
 * - IoTPConfig_setProperty
 * - IoTPConfig_getProperty
 * - IoTPConfig_clear
 */

int logCallbackActive = 0;
int logCallbackActiveExp = 1;

void logCallback (int level, char * message)
{
    fprintf(stdout, "%s\n", message? message:"NULL");
    fflush(stdout);
    logCallbackActive = 1;
}

/* Tests: Set Log handle */
int testConfig_setLogHandle(void)
{
    int rc = IOTPRC_SUCCESS;
    FILE *fd = NULL;
    int   fid = 0;

    rc = IoTPConfig_setLogHandler(IoTPLog_FilePointer, stdout);
    TEST_ASSERT("IoTPConfig_setLogHandler: FilePointer is stdout.", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setLogHandler(IoTPLog_FilePointer, stderr);
    TEST_ASSERT("IoTPConfig_setLogHandler: FilePointer is stderr.", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    logCallbackActive = 0;
    rc = IoTPConfig_setLogHandler(IoTPLog_Callback, logCallback);
    TEST_ASSERT("IoTPConfig_setLogHandler: Callback is valid.", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    LOG(INFO, "IoTPConfig_setLogHandler: Set callback.");
    TEST_ASSERT("IoTPConfig_setLogHandler: Validate Callback function.", logCallbackActive == 1, "rcE=%d rcA=%d", logCallbackActiveExp, logCallbackActive);

    rc = IoTPConfig_setLogHandler(IoTPLog_Callback, NULL);
    TEST_ASSERT("IoTPConfig_setLogHandler: Callback is NULL", rc == IOTPRC_PARAM_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_setLogHandler(IoTPLog_Callback, stdout);
    TEST_ASSERT("IoTPConfig_setLogHandler: Callback is stdout.", rc == IOTPRC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_INVALID_VALUE, rc);
    
    rc = IoTPConfig_setLogHandler(IoTPLog_FilePointer, logCallback);
    TEST_ASSERT("IoTPConfig_setLogHandler: FileDescriptor is Callback function.", rc == IOTPRC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_INVALID_VALUE, rc);

       
    if ((fd = fopen("test_cunit.log","a")) != NULL) {
        LOG(INFO, "Use test_cunit.log file");
        rc = IoTPConfig_setLogHandler(IoTPLog_FilePointer, fd);
        TEST_ASSERT("IoTPConfig_setLogHandler: FilePointer is valid.", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
        rc = IoTPConfig_setLogHandler(IoTPLog_FilePointer, stdout);
        fclose(fd);
        unlink("test_cunit.log");
    } else {
        LOG(ERROR, "Unable to open test_cunit.log. errno=%d", errno);
        rc = IOTPRC_FILE_OPEN;
    }
 
    rc = IoTPConfig_setLogHandler(IoTPLog_FileDescriptor, stdout);
    TEST_ASSERT("IoTPConfig_setLogHandler: FileDescriptor is stdout.", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setLogHandler(IoTPLog_FileDescriptor, stderr);
    TEST_ASSERT("IoTPConfig_setLogHandler: FileDescriptor is stderr.", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setLogHandler(IoTPLog_FileDescriptor, logCallback);
    TEST_ASSERT("IoTPConfig_setLogHandler: FileDescriptor is stderr.", rc == IOTPRC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_INVALID_VALUE, rc);


    if ((fid = open("test_cunit.log", O_RDWR | O_CREAT, 0666)) > 0 ) {
        LOG(INFO, "Use test_cunit.log file. fid=%d", fid);
        rc = IoTPConfig_setLogHandler(IoTPLog_FileDescriptor, &fid);
        TEST_ASSERT("IoTPConfig_setLogHandler: FileDescriptor is valid.", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
        rc = IoTPConfig_setLogHandler(IoTPLog_FilePointer, stdout);
        fclose(fd);
        unlink("test_cunit.log");
    } else {
        LOG(ERROR, "Unable to open test_cunit.log. errno=%d", errno);
        rc = IOTPRC_FILE_OPEN;
    }
 
    return rc;
}

/* Tests: Config object create */
int testConfig_create(void)
{
    int rc = IOTPRC_SUCCESS;

    IoTPConfig *config = NULL;

    rc = IoTPConfig_create(NULL, NULL);
    TEST_ASSERT("IoTPConfig_create: Init with NULL handle", rc == IOTPRC_PARAM_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_create(&config, NULL);
    TEST_ASSERT("IoTPConfig_create: Init with NULL file", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_create(&config, NULL);
    TEST_ASSERT("IoTPConfig_create: ReInit with NULL file", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPConfig_clear: Clear config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    config = NULL;
    rc = IoTPConfig_create(&config, "./wiotpdev.yaml");
    TEST_ASSERT("IoTPConfig_create: With valid file", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;

}

/* Tests: Config object clear */
int testConfig_clear(void)
{
    int rc = IOTPRC_SUCCESS;

    IoTPConfig *config = NULL;

    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPConfig_clear: Clear NULL config object", rc == IOTPRC_PARAM_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_create(&config, NULL);
    TEST_ASSERT("IoTPConfig_create: Init with NULL file", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPConfig_clear: Clear config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    return rc;
}

/* Tests: Config object set property */
int testConfig_setProperty(void)
{
    int rc = IOTPRC_SUCCESS;

    IoTPConfig *config = NULL;

    rc = IoTPConfig_create(&config, NULL);
    TEST_ASSERT("IoTPConfig_create: Init with NULL file", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(NULL, "platform.domain", "xxxx.com");
    TEST_ASSERT("IoTPConfig_setProperty: NULL config object", rc == IOTPRC_PARAM_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_setProperty(config, NULL, "xxxx.com");
    TEST_ASSERT("IoTPConfig_setProperty: NULL config param", rc == IOTPRC_INVALID_PARAM, "rcE=%d rcA=%d", IOTPRC_INVALID_PARAM, rc);

    rc = IoTPConfig_setProperty(config, "options.domain", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: options.domain is NULL", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "options.domain", "20");
    TEST_ASSERT("IoTPConfig_setProperty: options.domain is numeric", rc == IOTPRC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "options.domain", "internetofthings.ibmcloud.com");
    TEST_ASSERT("IoTPConfig_setProperty: options.domain is valid", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "options.mqtt.port", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: options.mqtt.port is NULL", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "options.mqtt.port", "xxxx");
    TEST_ASSERT("IoTPConfig_setProperty: options.mqtt.port is string", rc == IOTPRC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "options.mqtt.port", "23");
    TEST_ASSERT("IoTPConfig_setProperty: options.mqtt.port is 23", rc == IOTPRC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "options.mqtt.port", "-23");
    TEST_ASSERT("IoTPConfig_setProperty: options.mqtt.port is -23", rc == IOTPRC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "options.mqtt.port", "1883");
    TEST_ASSERT("IoTPConfig_setProperty: options.mqtt.port is valid - 8883", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "options.mqtt.port", "8883");
    TEST_ASSERT("IoTPConfig_setProperty: options.mqtt.port is valid - 8883", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "options.mqtt.port", "443");
    TEST_ASSERT("IoTPConfig_setProperty: options.mqtt.port is valid - 8883", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "options.mqtt.caFile", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: options.mqtt.caFile is NULL", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "options.mqtt.caFile", "20");
    TEST_ASSERT("IoTPConfig_setProperty: options.mqtt.caFile is numeric", rc == IOTPRC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "options.mqtt.caFile", "serverCertificate.pem");
    TEST_ASSERT("IoTPConfig_setProperty: options.mqtt.caFile is valid", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "identity.orgId", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: identity.orgId is NULL", rc == IOTPRC_PARAM_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "identity.orgId", "20");
    TEST_ASSERT("IoTPConfig_setProperty: identity.orgId is numeric", rc == IOTPRC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "identity.orgId", "xaa");
    TEST_ASSERT("IoTPConfig_setProperty: identity.orgId is valid", rc == IOTPRC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "identity.orgId", "xaaxxxx");
    TEST_ASSERT("IoTPConfig_setProperty: identity.orgId is valid length", rc == IOTPRC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "identity.orgId", "xxxxxx");
    TEST_ASSERT("IoTPConfig_setProperty: identity.orgId is valid", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "identity.typeId", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: identity.typeId is NULL", rc == IOTPRC_PARAM_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "identity.typeId", "devType1");
    TEST_ASSERT("IoTPConfig_setProperty: identity.typeId is valid", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "identity.deviceId", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: identity.deviceId is NULL", rc == IOTPRC_PARAM_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "identity.deviceId", "dev1");
    TEST_ASSERT("IoTPConfig_setProperty: identity.deviceId is valid", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "options.authMethod", "10");
    TEST_ASSERT("IoTPConfig_setProperty: options.authMethod is numeric", rc == IOTPRC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "options.authMethod", "xxxx");
    TEST_ASSERT("IoTPConfig_setProperty: options.authMethod is invalid", rc == IOTPRC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "options.authMethod", "token");
    TEST_ASSERT("IoTPConfig_setProperty: options.authMethod is valid - token", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "options.authMethod", "cert");
    TEST_ASSERT("IoTPConfig_setProperty: options.authMethod is valid - cert", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "options.authMethod", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: options.authMethod is valid - NULL", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "auth.token", "30");
    TEST_ASSERT("IoTPConfig_setProperty: auth.token is numeric", rc == IOTPRC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "auth.token", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: auth.token is NULL", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "auth.token", "dev1");
    TEST_ASSERT("IoTPConfig_setProperty: auth.token is valid", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "auth.keyStore", "30");
    TEST_ASSERT("IoTPConfig_setProperty: auth.keyStore is numeric", rc == IOTPRC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "auth.keyStore", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: auth.keyStore is NULL", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "auth.keyStore", "dev1");
    TEST_ASSERT("IoTPConfig_setProperty: auth.keyStore is valid", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "auth.privateKey", "30");
    TEST_ASSERT("IoTPConfig_setProperty: auth.privateKey is numeric", rc == IOTPRC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "auth.privateKey", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: auth.privateKey is NULL", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "auth.privateKey", "dev1");
    TEST_ASSERT("IoTPConfig_setProperty: auth.privateKey is valid", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "identity.appId", "30");
    TEST_ASSERT("IoTPConfig_setProperty: identity.appId is numeric", rc == IOTPRC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "identity.appId", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: identity.appId is NULL", rc == IOTPRC_PARAM_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "identity.appId", "dev1");
    TEST_ASSERT("IoTPConfig_setProperty: identity.appId is valid", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "auth.key", "30");
    TEST_ASSERT("IoTPConfig_setProperty: auth.key is numeric", rc == IOTPRC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "auth.key", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: auth.key is NULL", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "auth.key", "dev1");
    TEST_ASSERT("IoTPConfig_setProperty: auth.key is valid", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "auth.token", "30");
    TEST_ASSERT("IoTPConfig_setProperty: auth.token is numeric", rc == IOTPRC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "auth.token", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: auth.token is NULL", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "auth.token", "dev1");
    TEST_ASSERT("IoTPConfig_setProperty: auth.token is valid", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    return rc;
}

/* Tests: Config read config file */
int testConfig_readConfigFile(void)
{
    int rc = IOTPRC_SUCCESS;

    IoTPConfig *config = NULL;

    rc = IoTPConfig_create(&config, NULL);
    TEST_ASSERT("IoTPConfig_readConfigFile: Create config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_readConfigFile(NULL, NULL);
    TEST_ASSERT("IoTPConfig_readConfigFile: NULL config object", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);

    rc = IoTPConfig_readConfigFile(config, NULL);
    TEST_ASSERT("IoTPConfig_readConfigFile: NULL config file name", rc == IOTPRC_PARAM_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_readConfigFile(config, "/usr/sbin/xxxxx");
    TEST_ASSERT("IoTPConfig_readConfigFile: Invalid config file", rc == IOTPRC_FILE_OPEN, "rcE=%d rcA=%d", IOTPRC_FILE_OPEN, rc);

    rc = IoTPConfig_readConfigFile(config, "./wiotpdev.yaml");
    TEST_ASSERT("IoTPConfig_readConfigFile: Valid config file", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_readConfigFile(config, "./invalidconfig1.yaml");
    TEST_ASSERT("IoTPConfig_readConfigFile: Invalid config - catagory", rc == IOTPRC_INVALID_PARAM, "rcE=%d rcA=%d", IOTPRC_INVALID_PARAM, rc);

    rc = IoTPConfig_readConfigFile(config, "./invalidconfig2.yaml");
    TEST_ASSERT("IoTPConfig_readConfigFile: Invalid config - prop", rc == IOTPRC_INVALID_PARAM, "rcE=%d rcA=%d", IOTPRC_INVALID_PARAM, rc);

    rc = IoTPConfig_readConfigFile(config, "./invalidconfig3.yaml");
    TEST_ASSERT("IoTPConfig_readConfigFile: Invalid config - category prop", rc == IOTPRC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_PARAM_INVALID_VALUE, rc);

    return rc;
}


/* Tests: Config read config from env */
int testConfig_readEnvironment(void)
{
    int rc = IOTPRC_SUCCESS;
    char pval[1024];
    char *retval = pval;

    IoTPConfig *config = NULL;

    rc = IoTPConfig_create(&config, NULL);
    TEST_ASSERT("IoTPConfig_readEnvironment: Create config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_readEnvironment(NULL);
    TEST_ASSERT("IoTPConfig_readEnvironment: Invalid config object", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);

    setenv ("WIOTP_OPTIONS_DOMAIN", "test.com", 0);

    rc = IoTPConfig_readEnvironment(config);
    TEST_ASSERT("IoTPConfig_readEnvironment: Read env domain", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_getProperty(config, IoTPConfig_options_domain, &retval, 1024);
    TEST_ASSERT("IoTPConfig_readEnvironment: Read property - domain", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = strcmp(pval, "test.com");
    TEST_ASSERT("IoTPConfig_readEnvironment: verify property - domain", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_getProperty(config, IoTPConfig_options_mqtt_port, &retval, 1024);
    TEST_ASSERT("IoTPConfig_readEnvironment: Read property - port", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = strcmp(pval, "8883");
    TEST_ASSERT("IoTPConfig_readEnvironment: verify property - domain", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    return rc;
}



int main(int argc, char** argv)
{
    int rc = 0;
    int (*tests[])() = {testConfig_setLogHandle, testConfig_create, testConfig_clear, testConfig_setProperty, testConfig_readConfigFile, testConfig_readEnvironment};
    int i;
    int count = (int)TEST_COUNT(tests);

    testStart("IBM IoT Platform Client: Config API Tests", count);

    for (i = 0; i < count; i++) {
        printf("Run TestSuite:%d\n", i+1);
        tests[i]();
        printf("\n");
    }

    testEnd("IBM IoT Platform Client: Config API Tests", count);

    return rc;
}

