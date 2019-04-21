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
 *    Ranjan Dasgupt          - Initial API implementation
 *
 *******************************************************************************/

#if !defined(IOTP_RC_H_)
#define IOTP_RC_H_

#if defined(__cplusplus)
 extern "C" {
#endif

/*! \page iotpdevclient IoTP C Client Code Development Guide

TODO: Describe Client code development steps (including any best practices).

 */

/*! \page iotprc IoTP C Client Return codes

   Enumeration of return values from the IoTP_* functions defined in
   IBM Watson IoT Platform C Client SDK.
  
   - Value of 0 indicates a success response.
   - Value of 1 indicates a failure response.
   - Values of 1001 to 1500 are error codes returned by IoTP C client library.
  
   NOTE: IoTP_* APIs may return the following error codes or reported in the log messages:
  
         Values less than -1 are specific error codes returned by Paho MQTT C client. <br>
         - For details refer to https://www.eclipse.org/paho/files/mqttdoc/MQTTClient/html/_m_q_t_t_client_8h.html
  
         Any OS level errors will be reported in the log messages.

   TODO: Provide link to Error codes and messages.
  
 */


#if defined(WIN32) || defined(WIN64)
  #define DLLImport __declspec(dllimport)
  #define DLLExport __declspec(dllexport)
#else
  #define DLLImport extern
  #define DLLExport __attribute__ ((visibility ("default")))
#endif

/* IOTP Client reason and error codes */
typedef enum {

    /** Indicates successful completion of IoTP Client operation.  */
    IOTPRC_SUCCESS = 0,
    
    /** Indicates a generic failure an IoTP Client operation. */
    IOTPRC_FAILURE = 1,
    
    /** Memory error */
    IOTPRC_NOMEM = 1001,
    
    /** Could not open file. */
    IOTPRC_FILE_OPEN = 1002,
    
    /** Logging is already initialized. */
    IOTPRC_LOGGING_INITED = 1003,
    
    /** IoTP client handle is NULL or not initialized. */
    IOTPRC_INVALID_HANDLE = 1004,
    
    /** A required configuration parameter is not specified. */
    IOTPRC_MISSING_INPUT_PARAM = 1005,
    
    /** Invalid configuration parameter is specified. */
    IOTPRC_INVALID_PARAM = 1006,
    
    /** NULL or empty value for the configuration parameter is specified. */
    IOTPRC_PARAM_NULL_VALUE = 1007,
    
    /** Invalid value for the configuration parameter is specified. */
    IOTPRC_PARAM_INVALID_VALUE = 1008,
    
    /** WIoTP quickstart sandbox is not supported for the requested operation. */
    IOTPRC_QUICKSTART_NOT_SUPPORTED = 1009,
    
    /** Invalid argument is specified. */
    IOTPRC_INVALID_ARGS = 1010,
    
    /** NULL or empty value for an argument is specified. */
    IOTPRC_ARGS_NULL_VALUE = 1011,
    
    /** Invalid value of an argument is specified. */
    IOTPRC_ARGS_INVALID_VALUE = 1012,
    
    /** Error while executing the Client certificate callback */
    IOTPRC_CERT_CALLBACK = 1013,

    /** Can not destoy a handle which is in use */
    IOTPRC_HANDLE_IN_USE = 1014,

    /** File does not exist or can not access */
    IOTPRC_NOT_FOUND = 1015,

    /** IoTP Client is not connected to the platform */
    IOTPRC_NOT_CONNECTED = 1016,

    /** IoTP Client action timed out */
    IOTPRC_TIMEOUT = 1017,

    /** No message callback handler is configured. Can not process the message. */
    IOTPRC_HANDLER_NOT_FOUND = 1018,

    /** No message callback handler is invalid. */
    IOTPRC_HANDLER_INVALID = 1019,

    /** Device management action is initiated */
    IOTPRC_DM_ACTION_STARTED = 1020,

    /** Device management action has failed */
    IOTPRC_DM_ACTION_FAILED = 1021,

    /** Device management action is not supported */
    IOTPRC_DM_ACTION_NOT_SUPPORTED = 1022,

    /** Device management reponse parse error */
    IOTPRC_DM_RESPONSE_PARSE_ERROR = 1023,

    /** NULL request ID in device management reponse */
    IOTPRC_DM_RESPONSE_NULL_REQID = 1024,

    /** Invalid request ID in device management reponse */
    IOTPRC_DM_RESPONSE_INVALID_REQID = 1025,

    /** Callback is not set for device management action */
    IOTPRC_DM_ACTION_NO_CALLBACK = 1026

} IOTPRC;

/**
 * IoTPRC_toString() API returns string description of WIoTP client reason codes,
 * Paho MQTT Async client library error codes and Paho MQTT reason codes.
 *
 * @param rc             - Reason code
 * @return desc          - String with description of the reason code
 */
DLLExport const char * IOTPRC_toString(IOTPRC rc);

#if defined(__cplusplus)
 }
#endif

#endif /* IOTP_RC_H_ */

