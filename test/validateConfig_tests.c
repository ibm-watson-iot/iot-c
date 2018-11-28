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
    int rc = IoTP_SUCCESS;
    FILE *fd = NULL;
    int   fid = 0;

    rc = IoTPConfig_setLogHandler(IoTPLog_FilePointer, stdout);
    TEST_ASSERT("IoTPConfig_setLogHandler: FilePointer is stdout.", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setLogHandler(IoTPLog_FilePointer, stderr);
    TEST_ASSERT("IoTPConfig_setLogHandler: FilePointer is stderr.", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    logCallbackActive = 0;
    rc = IoTPConfig_setLogHandler(IoTPLog_Callback, logCallback);
    TEST_ASSERT("IoTPConfig_setLogHandler: Callback is valid.", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    LOG(INFO, "IoTPConfig_setLogHandler: Set callback.");
    TEST_ASSERT("IoTPConfig_setLogHandler: Validate Callback function.", logCallbackActive == 1, "rcE=%d rcA=%d", logCallbackActiveExp, logCallbackActive);

    rc = IoTPConfig_setLogHandler(IoTPLog_Callback, NULL);
    TEST_ASSERT("IoTPConfig_setLogHandler: Callback is NULL", rc == IoTP_RC_PARAM_NULL_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_setLogHandler(IoTPLog_Callback, stdout);
    TEST_ASSERT("IoTPConfig_setLogHandler: Callback is stdout.", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_INVALID_VALUE, rc);
    
    rc = IoTPConfig_setLogHandler(IoTPLog_FilePointer, logCallback);
    TEST_ASSERT("IoTPConfig_setLogHandler: FileDescriptor is Callback function.", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_INVALID_VALUE, rc);

       
    if ((fd = fopen("test_cunit.log","a")) != NULL) {
        LOG(INFO, "Use test_cunit.log file");
        rc = IoTPConfig_setLogHandler(IoTPLog_FilePointer, fd);
        TEST_ASSERT("IoTPConfig_setLogHandler: FilePointer is valid.", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
        rc = IoTPConfig_setLogHandler(IoTPLog_FilePointer, stdout);
        fclose(fd);
        unlink("test_cunit.log");
    } else {
        LOG(ERROR, "Unable to open test_cunit.log. errno=%d", errno);
        rc = IoTP_RC_FILE_OPEN;
    }
 
    rc = IoTPConfig_setLogHandler(IoTPLog_FileDescriptor, stdout);
    TEST_ASSERT("IoTPConfig_setLogHandler: FileDescriptor is stdout.", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setLogHandler(IoTPLog_FileDescriptor, stderr);
    TEST_ASSERT("IoTPConfig_setLogHandler: FileDescriptor is stderr.", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setLogHandler(IoTPLog_FileDescriptor, logCallback);
    TEST_ASSERT("IoTPConfig_setLogHandler: FileDescriptor is stderr.", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_INVALID_VALUE, rc);


    if ((fid = open("test_cunit.log", O_RDWR | O_CREAT, 0666)) > 0 ) {
        LOG(INFO, "Use test_cunit.log file. fid=%d", fid);
        rc = IoTPConfig_setLogHandler(IoTPLog_FileDescriptor, &fid);
        TEST_ASSERT("IoTPConfig_setLogHandler: FileDescriptor is valid.", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
        rc = IoTPConfig_setLogHandler(IoTPLog_FilePointer, stdout);
        fclose(fd);
        unlink("test_cunit.log");
    } else {
        LOG(ERROR, "Unable to open test_cunit.log. errno=%d", errno);
        rc = IoTP_RC_FILE_OPEN;
    }
 
    return rc;
}

/* Tests: Config object create */
int testConfig_create(void)
{
    int rc = IoTP_SUCCESS;

    IoTPConfig *config = NULL;

    rc = IoTPConfig_create(NULL, NULL);
    TEST_ASSERT("IoTPConfig_create: Init with NULL handle", rc == IoTP_RC_INVALID_HANDLE, "rcE=%d rcA=%d", IoTP_RC_INVALID_HANDLE, rc);

    rc = IoTPConfig_create(&config, NULL);
    TEST_ASSERT("IoTPConfig_create: Init with NULL file", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_create(&config, NULL);
    TEST_ASSERT("IoTPConfig_create: ReInit with NULL file", rc == IoTP_RC_INVALID_HANDLE, "rcE=%d rcA=%d", IoTP_RC_INVALID_HANDLE, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPConfig_clear: Clear config object", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    config = NULL;
    rc = IoTPConfig_create(&config, "./test/iotpclient.cfg");
    TEST_ASSERT("IoTPConfig_create: With valid file", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    return rc;

}

/* Tests: Config object clear */
int testConfig_clear(void)
{
    int rc = IoTP_SUCCESS;

    IoTPConfig *config = NULL;

    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPConfig_clear: Clear NULL config object", rc == IoTP_RC_INVALID_HANDLE, "rcE=%d rcA=%d", IoTP_RC_INVALID_HANDLE, rc);

    rc = IoTPConfig_create(&config, NULL);
    TEST_ASSERT("IoTPConfig_create: Init with NULL file", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPConfig_clear: Clear config object", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    return rc;
}

/* Tests: Config object set property */
int testConfig_setProperty(void)
{
    int rc = IoTP_SUCCESS;

    IoTPConfig *config = NULL;

    rc = IoTPConfig_create(&config, NULL);
    TEST_ASSERT("IoTPConfig_create: Init with NULL file", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(NULL, "platform.domain", "xxxx.com");
    TEST_ASSERT("IoTPConfig_setProperty: NULL config object", rc == IoTP_RC_INVALID_HANDLE, "rcE=%d rcA=%d", IoTP_RC_INVALID_HANDLE, rc);

    rc = IoTPConfig_setProperty(config, NULL, "xxxx.com");
    TEST_ASSERT("IoTPConfig_setProperty: NULL config param", rc == IoTP_RC_INVALID_PARAM, "rcE=%d rcA=%d", IoTP_RC_INVALID_PARAM, rc);

    rc = IoTPConfig_setProperty(config, "platform.domain", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: platform.domain is NULL", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "platform.domain", "20");
    TEST_ASSERT("IoTPConfig_setProperty: platform.domain is numeric", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "platform.domain", "internetofthings.ibmcloud.com");
    TEST_ASSERT("IoTPConfig_setProperty: platform.domain is valid", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "platform.port", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: platform.port is NULL", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "platform.port", "xxxx");
    TEST_ASSERT("IoTPConfig_setProperty: platform.port is string", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "platform.port", "23");
    TEST_ASSERT("IoTPConfig_setProperty: platform.port is 23", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "platform.port", "-23");
    TEST_ASSERT("IoTPConfig_setProperty: platform.port is -23", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "platform.port", "1883");
    TEST_ASSERT("IoTPConfig_setProperty: platform.port is valid - 8883", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "platform.port", "8883");
    TEST_ASSERT("IoTPConfig_setProperty: platform.port is valid - 8883", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "platform.port", "443");
    TEST_ASSERT("IoTPConfig_setProperty: platform.port is valid - 8883", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "platform.serverCertificatePath", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: platform.serverCertificatePath is NULL", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "platform.serverCertificatePath", "20");
    TEST_ASSERT("IoTPConfig_setProperty: platform.serverCertificatePath is numeric", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "platform.serverCertificatePath", "serverCertificate.pem");
    TEST_ASSERT("IoTPConfig_setProperty: platform.serverCertificatePath is valid", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "organization.id", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: organization.id is NULL", rc == IoTP_RC_PARAM_NULL_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "organization.id", "20");
    TEST_ASSERT("IoTPConfig_setProperty: organization.id is numeric", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "organization.id", "xaa");
    TEST_ASSERT("IoTPConfig_setProperty: organization.id is valid", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "organization.id", "xaaxxxx");
    TEST_ASSERT("IoTPConfig_setProperty: organization.id is valid length", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "organization.id", "xxxxxx");
    TEST_ASSERT("IoTPConfig_setProperty: organization.id is valid", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "device.typeId", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: device.typeId is NULL", rc == IoTP_RC_PARAM_NULL_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "device.typeId", "devType1");
    TEST_ASSERT("IoTPConfig_setProperty: device.typeId is valid", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "device.deviceId", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: device.deviceId is NULL", rc == IoTP_RC_PARAM_NULL_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "device.deviceId", "dev1");
    TEST_ASSERT("IoTPConfig_setProperty: device.deviceId is valid", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "device.authMethod", "10");
    TEST_ASSERT("IoTPConfig_setProperty: device.authMethod is numeric", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "device.authMethod", "xxxx");
    TEST_ASSERT("IoTPConfig_setProperty: device.authMethod is invalid", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "device.authMethod", "token");
    TEST_ASSERT("IoTPConfig_setProperty: device.authMethod is valid - token", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "device.authMethod", "cert");
    TEST_ASSERT("IoTPConfig_setProperty: device.authMethod is valid - cert", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "device.authMethod", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: device.authMethod is valid - NULL", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "device.authToken", "30");
    TEST_ASSERT("IoTPConfig_setProperty: device.authToken is numeric", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "device.authToken", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: device.authToken is NULL", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "device.authToken", "dev1");
    TEST_ASSERT("IoTPConfig_setProperty: device.authToken is valid", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "device.certificatePath", "30");
    TEST_ASSERT("IoTPConfig_setProperty: device.certificatePath is numeric", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "device.certificatePath", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: device.certificatePath is NULL", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "device.certificatePath", "dev1");
    TEST_ASSERT("IoTPConfig_setProperty: device.certificatePath is valid", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "device.keyPath", "30");
    TEST_ASSERT("IoTPConfig_setProperty: device.keyPath is numeric", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "device.keyPath", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: device.keyPath is NULL", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "device.keyPath", "dev1");
    TEST_ASSERT("IoTPConfig_setProperty: device.keyPath is valid", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);




    rc = IoTPConfig_setProperty(config, "gateway.typeId", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: gateway.typeId is NULL", rc == IoTP_RC_PARAM_NULL_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "gateway.typeId", "devType1");
    TEST_ASSERT("IoTPConfig_setProperty: gateway.typeId is valid", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "gateway.deviceId", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: gateway.deviceId is NULL", rc == IoTP_RC_PARAM_NULL_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "gateway.deviceId", "dev1");
    TEST_ASSERT("IoTPConfig_setProperty: gateway.deviceId is valid", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "gateway.authMethod", "10");
    TEST_ASSERT("IoTPConfig_setProperty: gateway.deviceId is numeric", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "gateway.authMethod", "xxxx");
    TEST_ASSERT("IoTPConfig_setProperty: gateway.deviceId is invalid", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "gateway.authMethod", "token");
    TEST_ASSERT("IoTPConfig_setProperty: gateway.deviceId is valid - token", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "gateway.authMethod", "cert");
    TEST_ASSERT("IoTPConfig_setProperty: gateway.deviceId is valid - cert", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "gateway.authMethod", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: gateway.deviceId is valid - NULL", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "gateway.authToken", "30");
    TEST_ASSERT("IoTPConfig_setProperty: gateway.authToken is numeric", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "gateway.authToken", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: gateway.authToken is NULL", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "gateway.authToken", "dev1");
    TEST_ASSERT("IoTPConfig_setProperty: gateway.authToken is valid", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "gateway.certificatePath", "30");
    TEST_ASSERT("IoTPConfig_setProperty: gateway.certificatePath is numeric", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "gateway.certificatePath", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: gateway.certificatePath is NULL", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "gateway.certificatePath", "dev1");
    TEST_ASSERT("IoTPConfig_setProperty: gateway.certificatePath is valid", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "gateway.keyPath", "30");
    TEST_ASSERT("IoTPConfig_setProperty: gateway.keyPath is numeric", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "gateway.keyPath", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: gateway.keyPath is NULL", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "gateway.keyPath", "dev1");
    TEST_ASSERT("IoTPConfig_setProperty: gateway.keyPath is valid", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "application.appId", "30");
    TEST_ASSERT("IoTPConfig_setProperty: application.appId is numeric", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "application.appId", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: application.appId is NULL", rc == IoTP_RC_PARAM_NULL_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "application.appId", "dev1");
    TEST_ASSERT("IoTPConfig_setProperty: application.appId is valid", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "application.APIKey", "30");
    TEST_ASSERT("IoTPConfig_setProperty: application.APIKey is numeric", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "application.APIKey", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: application.APIKey is NULL", rc == IoTP_RC_PARAM_NULL_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "application.APIKey", "dev1");
    TEST_ASSERT("IoTPConfig_setProperty: application.APIKey is valid", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_setProperty(config, "application.authToken", "30");
    TEST_ASSERT("IoTPConfig_setProperty: application.authToken is numeric", rc == IoTP_RC_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_INVALID_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "application.authToken", NULL);
    TEST_ASSERT("IoTPConfig_setProperty: application.authToken is NULL", rc == IoTP_RC_PARAM_NULL_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_setProperty(config, "application.authToken", "dev1");
    TEST_ASSERT("IoTPConfig_setProperty: application.authToken is valid", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    return rc;
}

/* Tests: Config read config file */
int testConfig_readConfigFile(void)
{
    int rc = IoTP_SUCCESS;

    IoTPConfig *config = NULL;

    rc = IoTPConfig_create(&config, NULL);
    TEST_ASSERT("IoTPConfig_readConfigFile: Create config", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_readConfigFile(NULL, NULL);
    TEST_ASSERT("IoTPConfig_readConfigFile: NULL config object", rc == IoTP_RC_INVALID_HANDLE, "rcE=%d rcA=%d", IoTP_RC_INVALID_HANDLE, rc);

    rc = IoTPConfig_readConfigFile(config, NULL);
    TEST_ASSERT("IoTPConfig_readConfigFile: NULL config file name", rc == IoTP_RC_PARAM_NULL_VALUE, "rcE=%d rcA=%d", IoTP_RC_PARAM_NULL_VALUE, rc);

    rc = IoTPConfig_readConfigFile(config, "/usr/sbin/xxxxx");
    TEST_ASSERT("IoTPConfig_readConfigFile: Invalid config file", rc == IoTP_RC_FILE_OPEN, "rcE=%d rcA=%d", IoTP_RC_FILE_OPEN, rc);

    rc = IoTPConfig_readConfigFile(config, "./test/iotpclient.cfg");
    TEST_ASSERT("IoTPConfig_readConfigFile: Valid config file", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_readConfigFile(config, "./test/invalidconfig1.cfg");
    TEST_ASSERT("IoTPConfig_readConfigFile: Invalid config - catagory", rc == IoTP_RC_INVALID_PARAM, "rcE=%d rcA=%d", IoTP_RC_INVALID_PARAM, rc);

    rc = IoTPConfig_readConfigFile(config, "./test/invalidconfig2.cfg");
    TEST_ASSERT("IoTPConfig_readConfigFile: Invalid config - prop", rc == IoTP_RC_INVALID_PARAM, "rcE=%d rcA=%d", IoTP_RC_INVALID_PARAM, rc);

    rc = IoTPConfig_readConfigFile(config, "./test/invalidconfig3.cfg");
    TEST_ASSERT("IoTPConfig_readConfigFile: Invalid config - category prop", rc == IoTP_RC_INVALID_PARAM, "rcE=%d rcA=%d", IoTP_RC_INVALID_PARAM, rc);

    return rc;
}


/* Tests: Config read config from env */
int testConfig_readEnvironment(void)
{
    int rc = IoTP_SUCCESS;
    char pval[1024];
    char *retval = pval;

    IoTPConfig *config = NULL;

    rc = IoTPConfig_create(&config, NULL);
    TEST_ASSERT("IoTPConfig_readEnvironment: Create config", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_readEnvironment(NULL);
    TEST_ASSERT("IoTPConfig_readEnvironment: Invalid config object", rc == IoTP_RC_INVALID_HANDLE, "rcE=%d rcA=%d", IoTP_RC_INVALID_HANDLE, rc);

    setenv ("IoTPConfig.Platform.domain", "test.com", 0);

    rc = IoTPConfig_readEnvironment(config);
    TEST_ASSERT("IoTPConfig_readEnvironment: Read env domain", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_getProperty(config, IoTPConfig_Platform_domain, &retval, 1024);
    TEST_ASSERT("IoTPConfig_readEnvironment: Read property - domain", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = strcmp(pval, "test.com");
    TEST_ASSERT("IoTPConfig_readEnvironment: verify property - domain", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_getProperty(config, IoTPConfig_Platform_port, &retval, 1024);
    TEST_ASSERT("IoTPConfig_readEnvironment: Read property - port", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = strcmp(pval, "8883");
    TEST_ASSERT("IoTPConfig_readEnvironment: verify property - domain", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

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

