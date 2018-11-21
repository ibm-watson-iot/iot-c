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

#include "iotp_mqttclient.h"
#include "test_utils.h"

/*
 * validate.c: IBM Watson IoT Platform C Client API validation tests
 *
 * This file contains test functions to test validation of following APIs:
 * - IoTPClient_initLogging
 * - IoTPClient_stopLogging
 * - IoTPClient_initialize
 * - IoTPClient_destroy
 */

/* Tests for Logging APIs */
int test_logging(void)
{
    int rc = IoTP_SUCCESS;

    /* Init logging - valid cases */
    rc = IoTPClient_initLogging(LOGLEVEL_ERROR, NULL);
    TEST_ASSERT("IoTPClient_initLogging: Init with logLevel LOGLEVEL_ERROR.", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    rc = IoTPClient_stopLogging();
    rc = IoTPClient_initLogging(LOGLEVEL_WARN, NULL);
    TEST_ASSERT("IoTPClient_initLogging: Init with logLevel LOGLEVEL_WARN.", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    rc = IoTPClient_stopLogging();
    rc = IoTPClient_initLogging(LOGLEVEL_INFO, NULL);
    TEST_ASSERT("IoTPClient_initLogging: Init with logLevel LOGLEVEL_INFO.", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    rc = IoTPClient_stopLogging();
    rc = IoTPClient_initLogging(LOGLEVEL_DEBUG, NULL);
    TEST_ASSERT("IoTPClient_initLogging: Init with logLevel LOGLEVEL_DEBUG.", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    rc = IoTPClient_stopLogging();
    rc = IoTPClient_initLogging(LOGLEVEL_TRACE, NULL);
    TEST_ASSERT("IoTPClient_initLogging: Init with logLevel LOGLEVEL_TRACE.", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    rc = IoTPClient_stopLogging();
    rc = IoTPClient_initLogging(LOGLEVEL_TRACE, "/tmp/iotpclient.log");
    TEST_ASSERT("IoTPClient_initLogging: Init with valid log file /tmp/iotpclient.log.", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    rc = IoTPClient_stopLogging();
    TEST_ASSERT("IoTPClient_stopLogging: Stop logging.", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    /* Init logging - invalid cases */
    rc = IoTPClient_initLogging(LOGLEVEL_ERROR, NULL);
    rc = IoTPClient_initLogging(LOGLEVEL_ERROR, NULL);
    TEST_ASSERT("IoTPClient_initLogging: Reinit logging.", rc == IoTP_ERROR_LOGGING_INITED, "rcE=%d rcA=%d", IoTP_ERROR_LOGGING_INITED, rc);
    rc = IoTPClient_stopLogging();
    rc = IoTPClient_initLogging(0, NULL);
    TEST_ASSERT("IoTPClient_initLogging: Init with invalid logLevel < LOGLEVEL_ERROR.", rc == IoTP_ERROR_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_ERROR_PARAM_INVALID_VALUE, rc);
    rc = IoTPClient_initLogging(6, NULL);
    TEST_ASSERT("IoTPClient_initLogging: Init with invalid logLevel > LOGLEVEL_TRACE.", rc == IoTP_ERROR_PARAM_INVALID_VALUE, "rcE=%d rcA=%d", IoTP_ERROR_PARAM_INVALID_VALUE, rc);
    rc = IoTPClient_initLogging(LOGLEVEL_ERROR, "/root/xxxx/yyyy");
    TEST_ASSERT("IoTPClient_initLogging: Init with invalid file /root/xxxx/yyyy.", rc == IoTP_ERROR_FILE_OPEN, "rcE=%d rcA=%d", IoTP_ERROR_FILE_OPEN, rc);
    rc = IoTPClient_stopLogging();
    TEST_ASSERT("IoTPClient_stopLogging: Logging in not initiliazed.", rc == IoTP_ERROR_LOGGING_INITED, "rcE=%d rcA=%d", IoTP_ERROR_LOGGING_INITED, rc);

    return rc;
}


/* Tests for client handle init and destroy APIs */
int test_client_handle(void)
{
    int rc = IoTP_SUCCESS;

    /* Valid IoTPClient handle */
    IoTPClient *client = NULL;
    rc = IoTPClient_initialize(&client);    
    TEST_ASSERT("IoTPClient_initialize: Initialize a handle.", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    if (client == NULL) rc = IoTP_ERROR_INVALID_HANDLE;
    TEST_ASSERT("IoTPClient_initialize: Handle is not NULL.", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    /* Invalid IoTPClient handle - reinit handle */
    rc = IoTPClient_initialize(&client);    
    TEST_ASSERT("IoTPClient_initialize: Reinitialize handle.", rc == IoTP_ERROR_INVALID_HANDLE, "rcE=%d rcA=%d", IoTP_ERROR_INVALID_HANDLE, rc);

    /* Destroy invalid handle */
    rc = IoTPClient_destroy(NULL);
    TEST_ASSERT("IoTPClient_destroy: Invalid handle.", rc == IoTP_ERROR_INVALID_HANDLE, "rcE=%d rcA=%d", IoTP_ERROR_INVALID_HANDLE, rc);

    /* Destroy valid handle */
    rc = IoTPClient_destroy(client);
    TEST_ASSERT("IoTPClient_destroy: Destroy a valid handle.", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    return rc;
}

int main(int argc, char** argv)
{
    int rc = 0;
    int (*tests[])() = {test_logging, test_client_handle};
    int i;
    int count = (int)TEST_COUNT(tests);

    testStart("IBM IoT Platform Client: API Tests", count);

    for (i = 0; i < count; i++) {
        printf("Run TestSuite:%d\n", i);
        tests[i]();
        printf("\n");
    }

    testEnd();

    return rc;
}

