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
 * Contributors:
 *    Ranjan Dasgupta - Initial drop of gatewaySample.c
 * 
 *******************************************************************************/

/*
 * This sample shows how-to develop a gateway code using Watson IoT Platform
 * iot-c gateway client library, connect and interact with Watson IoT Platform Service.
 * 
 * This sample includes the function/code snippets to perform the following actions:
 * - Initiliaze client library
 * - Configure gateway from configuration parameters specified in a configuration file
 * - Set client logging
 * - Enable error handling routines
 * - Send gateway events to WIoTP service
 * - Receive and process commands from WIoTP service
 *
 * SYNTAX:
 * gatewaySample --config <config_file_path>
 *
 * Pre-requisite:
 * 1. This sample requires a gateway configuration file. 
 *    Refer to "Gateway Confioguration File" section of iot-c docs for details.
 * 2. Register type and id (specified in the configuration file) with IBM Watson IoT Platform service.
 *
 */

#include <stdio.h>
#include <signal.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>

/* Include header file of IBM Watson IoT platform C Client for gateways */ 
#include "iotp_gateway.h"

char *configFilePath = NULL;
volatile int interrupt = 0;
char *progname = "gatewaySample";

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
 * Gateway command callback function
 * Gateway developers can customize this function based on their use case
 * to handle gateway commands sent by WIoTP.
 * Set this callback function using API setCommandHandler().
 */
void  gatewayCommandCallback (char* type, char* id, char* commandName, char *format, void* payload, size_t payloadSize)
{
    if ( type == NULL && id == NULL ) {
        fprintf(stdout, "Received gateway management status:");

    } else {
        fprintf(stdout, "Received gateway command:\n");
        fprintf(stdout, "Type=%s ID=%s CommandName=%s Format=%s Len=%d\n", type, id, commandName, format, (int)payloadSize);
        fprintf(stdout, "Payload: %s\n", (char *)payload);
    }

    /* Gateway developers - add your custom code to process gateway commands */
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
    IoTPGateway *gateway = NULL;

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

    /* Create IoTPGateway object */
    rc = IoTPGateway_create(&gateway, config);
    if ( rc != 0 ) {
        fprintf(stderr, "ERROR: Failed to configure IoTP gateway: rc=%d\n", rc);
        exit(1);
    }

    /* Set MQTT Trace handler */
    rc = IoTPGateway_setMQTTLogHandler(gateway, &MQTTTraceCallback);
    if ( rc != 0 ) {
        fprintf(stderr, "WARN: Failed to set MQTT Trace handler: rc=%d\n", rc);
    }

    /* Invoke connection API IoTPGateway_connect() to connect to WIoTP. */
    rc = IoTPGateway_connect(gateway);
    if ( rc != 0 ) {
        fprintf(stderr, "ERROR: Failed to connect to Watson IoT Platform: rc=%d\n", rc);
        exit(1);
    }

    /*
     * Set gateway command callback using API IoTPGateway_setCommandHandler().
     * Refer to gatewayCommandCallback() function DEV_NOTES for details on
     * how to process gateway commands received from WIoTP.
     */
    IoTPGateway_setCommandHandler(gateway, gatewayCommandCallback);

    /*
     * Invoke gateway command subscription API IoTPGateway_subscribeToCommands().
     * The arguments for this API are commandName, format, QoS
     * If you want to subscribe to all commands of any format, set commandName and format to "+"
     */
    char *commandName = "+";
    char *format = "+";
    IoTPGateway_subscribeToCommands(gateway, commandName, format);


    /* Use IoTPGateway_sendEvent() API to send gateway events to Watson IoT Platform. */

    /* Sample event - this sample gateway will send this event once in every 10 seconds. */
    char *data = "{\"d\" : {\"SensorID\": \"Test\", \"Reading\": 7 }}";

    while(!interrupt)
    {
        fprintf(stdout, "Send status event\n");
        rc = IoTPGateway_sendEvent(gateway,"status",data, "json", QoS0, NULL);
        fprintf(stdout, "RC from publishEvent(): %d\n", rc);
        sleep(10);
    }

    fprintf(stdout, "Received a signal - exiting publish event cycle.\n");

    /* Disconnect gateway */
    rc = IoTPGateway_disconnect(gateway);
    if ( rc != IoTP_SUCCESS ) {
        fprintf(stderr, "ERROR: Failed to disconnect from  Watson IoT Platform: rc=%d\n", rc);
        exit(1);
    }

    /* Destroy client */
    IoTPGateway_destroy(gateway);

    /* Clear configuration */
    IoTPConfig_clear(config);

    return 0;

}

