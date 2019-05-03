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

/*! \page iotplog Logging and Tracing

```
This page is still under construction.
```

*/

/*! \page iotprc C Client SDK Return codes

The IoTP_* functions defined in IBM Watson IoT Platform C Client SDK retrurns the following codes:
  
- Value of 0 indicates a success response.
- Value of 1 indicates a failure response.
- Values of 1001 to 1500 are {@link IOTPRC} codes returned by IoTP C Client libraries. 
- Values less than -1 are specific error codes returned by Paho MQTT C client. For details on
  refer to <a href="https://www.eclipse.org/paho/files/mqttdoc/MQTTAsync/html/_m_q_t_t_reason_codes_8h.html" target="_blank">MQTTReasonCodes</a> in Paho Asynchronous MQTT C Client Library.

*/


#if defined(WIN32) || defined(WIN64)
  #define DLLImport __declspec(dllimport)
  #define DLLExport __declspec(dllexport)
#else
  #define DLLImport extern
  #define DLLExport __attribute__ ((visibility ("default")))
#endif

/**
 * IOTP Client reason and error codes 
 */
typedef enum {

    /** 0: WIoTP client oprration has completed successfully. */
    IOTPRC_SUCCESS = 0,
    
    /** 1: WIoTP client oprration has failed to complete. */
    IOTPRC_FAILURE = 1,
    
    /** 1001: System memory is not available. */
    IOTPRC_NOMEM = 1001,
    
    /** 1002: Could not open the specified file. */
    IOTPRC_FILE_OPEN = 1002,
    
    /** 1003: Logging is already initialized. */
    IOTPRC_LOGGING_INITED = 1003,
    
    /** 1004: WIoTP client handle or configuration handle, is NULL or not initialized. */
    IOTPRC_INVALID_HANDLE = 1004,
    
    /** 1005: A required configuration item is not specified. */
    IOTPRC_MISSING_INPUT_PARAM = 1005,
    
    /** 1006: An invalid configuration item is specified. */
    IOTPRC_INVALID_PARAM = 1006,
    
    /** 1007: The value specified for a configuration item is NULL or empty. */
    IOTPRC_PARAM_NULL_VALUE = 1007,
    
    /** 1008: The value specified for a configuration item is not valid. */
    IOTPRC_PARAM_INVALID_VALUE = 1008,
    
    /** 1009: WIoTP quickstart sandbox is not supported for the requested operation. */
    IOTPRC_QUICKSTART_NOT_SUPPORTED = 1009,
    
    /** 1010: An invalid argument is specified for the API. */
    IOTPRC_INVALID_ARGS = 1010,
    
    /** 1011: The argument value for the API is NULL or empty value. */
    IOTPRC_ARGS_NULL_VALUE = 1011,
    
    /** 1012: The specified argument value for the API is not valid. */
    IOTPRC_ARGS_INVALID_VALUE = 1012,
   
    /** 1013: The client certificate callback failed with an error. */
    IOTPRC_CERT_CALLBACK = 1013,

    /** 1014: The handle is in use. */
    IOTPRC_HANDLE_IN_USE = 1014,

    /** 1015: The file does not exist or is not accessible. */
    IOTPRC_NOT_FOUND = 1015,

    /** 1016: WIoTP client is not connected to the platform. */
    IOTPRC_NOT_CONNECTED = 1016,

    /** 1017: WIoTP client action failed to complete in configured time. */
    IOTPRC_TIMEOUT = 1017,

    /** 1018: No message callback handler is configured. Can not process the message. */
    IOTPRC_HANDLER_NOT_FOUND = 1018,

    /** 1019: An invalid callback handler is specified. */
    IOTPRC_HANDLER_INVALID = 1019,

    /** 1020: Device management action has started. */
    IOTPRC_DM_ACTION_STARTED = 1020,

    /** 1021: Device management action has failed to start. */
    IOTPRC_DM_ACTION_FAILED = 1021,

    /** 1022: Device management action is not supported. */
    IOTPRC_DM_ACTION_NOT_SUPPORTED = 1022,

    /** 1023: Could not parse device management response from WIoTP. */
    IOTPRC_DM_RESPONSE_PARSE_ERROR = 1023,

    /** 1024: Received a NULL request ID from WIoTP. */
    IOTPRC_DM_RESPONSE_NULL_REQID = 1024,

    /** 1025: Received request ID does not match with cached requiest ID. */
    IOTPRC_DM_RESPONSE_INVALID_REQID = 1025,

    /** 1026: Could not find a call callback for the device management action. */
    IOTPRC_DM_ACTION_NO_CALLBACK = 1026

} IOTPRC;

/**
 * IOTPRC_toString() API returns string description of WIoTP client reason codes,
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

