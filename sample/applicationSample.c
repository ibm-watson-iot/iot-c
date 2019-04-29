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
 * Contributors:
 *    Ranjan Dasgupta - Initial drop of applicationSample.c
 * 
 *******************************************************************************/

/*
 * This sample shows how-to develop a application code using Watson IoT Platform
 * iot-c application client library, connect and interact with Watson IoT Platform Service.
 * 
 * This sample includes the function/code snippets to perform the following actions:
 * - Initiliaze client library
 * - Configure application from configuration parameters specified in a configuration file
 * - Set client logging
 * - Enable error handling routines
 * - Send application events to WIoTP service
 * - Receive and process commands from WIoTP service
 *
 * SYNTAX:
 * applicationSample --config <config_file_path>
 *
 * Pre-requisite:
 * 1. This sample requires a application configuration file. 
 *    Refer to "Application Confioguration File" section of iot-c docs for details.
 * 2. Register type and id (specified in the configuration file) with IBM Watson IoT Platform service.
 *
 */

#include <stdio.h>
#include <signal.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>

/* Include header file of IBM Watson IoT platform C Client for applications */ 
#include "iotp_application.h"

char *configFilePath = NULL;
volatile int interrupt = 0;
char *progname = "applicationSample";

/* Usage text */
void usage(void) {
    fprintf(stderr, "Usage: %s --config config_file_path\n", progname);
    exit(1);
}

/* Signal handler - to support CTRL-C to quit */
void sigHandler(int signo) {
    signal(SIGINT, NULL);
    fprintf(stdout, "Received signal: %d\n", signo);
    interrupt = 1;
}

/* Get and process command line options */
void getopts(int argc, char** argv)
{
    int count = 1;

    while (count < argc)
    {
        if (strcmp(argv[count], "--config") == 0)
        {
            if (++count < argc)
                configFilePath = argv[count];
            else
                usage();
        }
        count++;
    }
}

/* 
 * Application command callback function
 * Application developers can customize this function based on their use case
 * to handle application commands sent by WIoTP.
 * Set this callback function using API setCommandHandler().
 */
void  applicationCommandCallback (char* type, char* id, char* commandName, char *format, void* payload, size_t payloadSize)
{
    fprintf(stdout, "Received application command:\n");
    fprintf(stdout, "Type=%s ID=%s CommandName=%s Format=%s Len=%d\n", type, id, commandName, format, (int)payloadSize);
    fprintf(stdout, "Payload: %s\n", (char *)payload);

    /* Application developers - add your custom code to process application commands */
}

void logCallback (int level, char * message)
{
    if ( level > 0 ) 
        fprintf(stdout, "%s\n", message? message:"NULL");
    fflush(stdout);
}

void MQTTTraceCallback (int level, char * message)
{
    if ( level > 0 ) 
        fprintf(stdout, "%s\n", message? message:"NULL");
    fflush(stdout);
}

/* Main program */
int main(int argc, char *argv[])
{
    int rc = 0;

    /* 
     * DEV_NOTES:
     * Specifiy variable for WIoT client object 
     */
    IoTPConfig *config = NULL;
    IoTPApplication *application = NULL;

    /* check for args */
    if ( argc < 2 )
        usage();

    /* Set signal handlers */
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    /* get argument options */
    getopts(argc, argv);

    /* Set IoTP Client log handler */
    rc = IoTPConfig_setLogHandler(IoTPLog_FileDescriptor, stdout);
    if ( rc != 0 ) {
        fprintf(stderr, "WARN: Failed to set IoTP Client log handler: rc=%d\n", rc);
    }

    /* Create IoTPConfig object using configuration options defined in the configuration file. */
    rc = IoTPConfig_create(&config, configFilePath);
    if ( rc != 0 ) {
        fprintf(stderr, "ERROR: Failed to initialize configuration: rc=%d\n", rc);
        exit(1);
    }

    /* Create IoTPApplication object */
    rc = IoTPApplication_create(&application, config);
    if ( rc != 0 ) {
        fprintf(stderr, "ERROR: Failed to configure IoTP application: rc=%d\n", rc);
        exit(1);
    }

    /* Set MQTT Trace handler */
    rc = IoTPApplication_setMQTTLogHandler(application, &MQTTTraceCallback);
    if ( rc != 0 ) {
        fprintf(stderr, "WARN: Failed to set MQTT Trace handler: rc=%d\n", rc);
    }

    /* Invoke connection API IoTPApplication_connect() to connect to WIoTP. */
    rc = IoTPApplication_connect(application);
    if ( rc != 0 ) {
        fprintf(stderr, "ERROR: Failed to connect to Watson IoT Platform: rc=%d Reason: %s\n", rc, IOTPRC_toString(rc));
        exit(1);
    }

    /*
     * Set application command callback using API IoTPApplication_setCommandHandler().
     * Refer to applicationCommandCallback() function for details on
     * how to process application commands received from WIoTP.
     */
    char *devType = "+";
    char *devId = "+";
    char *commandName = "+";
    char *format = "+";
    IoTPApplication_setCommandHandler(application, applicationCommandCallback, devType, devId, commandName, format);

    /*
     * Invoke application command subscription API IoTPApplication_subscribeToCommands().
     * The arguments for this API are commandName, format, QoS
     * If you want to subscribe to all commands of any format, set commandName and format to "+"
     */
    IoTPApplication_subscribeToCommands(application, devType, devId, commandName, format);


    /* Use IoTPApplication_sendEvent() API to send application events to Watson IoT Platform. */

    /* Sample event - this sample application will send this event once in every 10 seconds. */
    char *data = "{\"d\" : {\"SensorID\": \"Test\", \"Reading\": 7 }}";

    while(!interrupt)
    {
        fprintf(stdout, "Send status event\n");
        rc = IoTPApplication_sendEvent(application,"devType1","dev1","status","json", data , QoS0, NULL);
        fprintf(stdout, "RC from publishEvent(): %d\n", rc);
        sleep(10);
    }

    fprintf(stdout, "Received a signal - exiting publish event cycle.\n");

    /* Disconnect application */
    rc = IoTPApplication_disconnect(application);
    if ( rc != IOTPRC_SUCCESS ) {
        fprintf(stderr, "ERROR: Failed to disconnect from  Watson IoT Platform: rc=%d\n", rc);
        exit(1);
    }

    /* Destroy client */
    IoTPApplication_destroy(application);

    /* Clear configuration */
    IoTPConfig_clear(config);

    return 0;

}

