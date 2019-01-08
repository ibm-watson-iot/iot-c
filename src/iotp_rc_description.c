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

#include "iotp_utils.h"

static struct {
    IoTP_RC      rc;
    const char * desc;
} rcDesc[] = {
    { IoTP_SUCCESS,                     "WIoTP client oprration has completed successfully." },
    { IoTP_FAILURE,                     "WIoTP client oprration has failed to complete." },
    { IoTP_RC_NOMEM,                    "System memory is not available." },
    { IoTP_RC_FILE_OPEN,                "Could not open the specified file." },
    { IoTP_RC_LOGGING_INITED,           "Logging is already initialized." },
    { IoTP_RC_INVALID_HANDLE,           "WIoTP client handle is NULL or not initialized." },
    { IoTP_RC_MISSING_INPUT_PARAM,      "A required configuration parameter is not specified." },
    { IoTP_RC_INVALID_PARAM,            "An invalid configuration parameter is specified." },
    { IoTP_RC_PARAM_NULL_VALUE,         "NULL or empty value for the configuration parameter is specified." },
    { IoTP_RC_PARAM_INVALID_VALUE,      "The value specified for the configuration parameter is not valid." },
    { IoTP_RC_QUICKSTART_NOT_SUPPORTED, "WIoTP quickstart sandbox is not supported for the requested operation." },
    { IoTP_RC_INVALID_ARGS,             "An invalid argument is specified for the API." },
    { IoTP_RC_ARGS_NULL_VALUE,          "The argument value for the API is NULL or empty value." },
    { IoTP_RC_ARGS_INVALID_VALUE,       "The specified argument value for the API is invalid." },
    { IoTP_RC_CERT_CALLBACK,            "The client certificate callback failed with an error." },
    { IoTP_RC_HANDLE_IN_USE,            "The handle is in use." },
    { IoTP_RC_NO_ACCESS,                "The file does not exist or is not accessible." },
    { IoTP_RC_NOT_CONNECTED,            "WIoTP client is not connected to the platform." },
    { IoTP_RC_TIMEOUT,                  "WIoTP client action failed to complete in configured time." }
};

#define NUM_RC (sizeof(rcDesc) / sizeof(rcDesc[0]))


/*
 * Returns WIoTP client reason code description.
 */
const char * IoTP_rc_description(IoTP_RC rc)
{
    int i = 0;
    const char *desc = NULL;

    for (i=0; i<NUM_RC; i++) {
        if (rcDesc[i].rc == rc) {
            desc = rcDesc[i].desc;
            break;
        }
    }

    return desc;
}


