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
 *    Ranjan Dasgupt          - Initial API implementation
 *
 *******************************************************************************/

#if !defined(IOTP_RC_H_)
#define IOTP_RC_H_

#if defined(__cplusplus)
 extern "C" {
#endif

/*
/// @cond EXCLUDE
*/

#if defined(WIN32) || defined(WIN64)
  #define DLLImport __declspec(dllimport)
  #define DLLExport __declspec(dllexport)
#else
  #define DLLImport extern
  #define DLLExport __attribute__ ((visibility ("default")))
#endif

/*
/// @endcond
*/


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

typedef enum {

    /** Indicates successful completion of IoTP Client operation.  */
    IoTP_SUCCESS = 0,
    
    /** Indicates a generic failure an IoTP Client operation. */
    IoTP_FAILURE = 1,
    
    /** Memory error */
    IoTP_RC_NOMEM = 1001,
    
    /** Could not open file. */
    IoTP_RC_FILE_OPEN = 1002,
    
    /** Logging is already initialized. */
    IoTP_RC_LOGGING_INITED = 1003,
    
    /** IoTP client handle is NULL or not initialized. */
    IoTP_RC_INVALID_HANDLE = 1004,
    
    /** A required configuration parameter is not specified. */
    IoTP_RC_MISSING_INPUT_PARAM = 1005,
    
    /** Invalid configuration parameter is specified. */
    IoTP_RC_INVALID_PARAM = 1006,
    
    /** NULL or empty value for the configuration parameter is specified. */
    IoTP_RC_PARAM_NULL_VALUE = 1007,
    
    /** Invalid value for the configuration parameter is specified. */
    IoTP_RC_PARAM_INVALID_VALUE = 1008,
    
    /** WIoTP quickstart sandbox is not supported for the requested operation. */
    IoTP_RC_QUICKSTART_NOT_SUPPORTED = 1009,
    
    /** Invalid argument is specified. */
    IoTP_RC_INVALID_ARGS = 1010,
    
    /** NULL or empty value for an argument is specified. */
    IoTP_RC_ARGS_NULL_VALUE = 1011,
    
    /** Invalid value of an argument is specified. */
    IoTP_RC_ARGS_INVALID_VALUE = 1012,
    
    /** Error while executing the Client certificate callback */
    IoTP_RC_CERT_CALLBACK = 1013,

    /** Can not destoy a handle which is in use */
    IoTP_RC_HANDLE_IN_USE = 1014,

    /** File does not exist or can not access */
    IoTP_RC_NO_ACCESS = 1015,

    /** IoTP Client is not connected to the platform */
    IoTP_RC_NOT_CONNECTED = 1016

} IoTP_RC;


#if defined(__cplusplus)
 }
#endif

#endif /* IOTP_RC_H_ */

