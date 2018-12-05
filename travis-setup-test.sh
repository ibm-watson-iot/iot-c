#!/bin/bash
#*******************************************************************************
#  Copyright (c) 2018 IBM Corp.
#
#  All rights reserved. This program and the accompanying materials
#  are made available under the terms of the Eclipse Public License v1.0
#  and Eclipse Distribution License v1.0 which accompany this distribution.
#
#  The Eclipse Public License is available at
#     http://www.eclipse.org/legal/epl-v10.html
#  and the Eclipse Distribution License is available at
#    http://www.eclipse.org/org/documents/edl-v10.php.
#
#  Contributors:
#     Ranjan Dasgupta - initial drop of WIoTP samples for NXP i.MX
#
#*******************************************************************************/
#
# Test setup
#
# Creates set of device types and devices in WIoTP service.
#

# Organization ID, API Key and Token are from encrypted environment variables in travis.
#
ORG="${orgid}"
APIKEY="${apikey}"
APITOKEN="${apitoken}"
DEVICETOKEN="${devtoken}"

# 
# To use the script in your own test environment, logon to Watson IoT platform 
# dashboard to create API key, uncomment and set the following variables:
#
# ORG="xxxxxx"
# APIKEY="a-xxxxxx-xxxxxxxxxx"
# APITOKEN="xxxxxxxxxxxxxxxxxx"
# DEVTOKEN="iotTestPassw0rd"

# Specify number of device types.
# Script will add device types: iot_test_devType1, iot_test_devType2, ...
NODEVTYPE=2

# Specify number of devices of each types
# Script will add devices: iot_test_dev1, iot_test_dev2, ...
NODEV=5

echo
echo "Run tests using WiOTP instance ${ORG}"
echo

# Set for verbose output
# VERBOSE="-v"

#
# Create Device Types
#
i=1
while [ $i -le $NODEVTYPE ]
do
    devType="iotc_test_devType${i}"
    echo "Create device type: ${devType}"
    PAYLOAD="{\"id\":\"${devType}\",\"classId\":\"Device\",\"deviceInfo\":{},\"metadata\":{}}"
    # echo ${PAYLOAD}
    curl -k -u "${APIKEY}:${APITOKEN}" --request POST \
         --url https://${ORG}.internetofthings.ibmcloud.com/api/v0002/device/types \
         --header 'content-type: application/json' \
         --data "${PAYLOAD}" ${VERBOSE}
    echo
    echo
    ((i = i + 1 ))

    #
    # Create Devices
    #
    j=1
    while [ $j -le $NODEV ]
    do
        dev="iotc_test_dev${j}"
        echo "Create device: ${dev}"
        PAYLOAD="{\"deviceId\":\"${dev}\",\"authToken\":\"${DEVTOKEN}\",\"deviceInfo\":{},\"location\":{},\"metadata\":{}}"
        curl -k -u "${APIKEY}:${APITOKEN}" --request POST \
            --url https://${ORG}.internetofthings.ibmcloud.com/api/v0002/device/types/${devType}/devices \
            --header 'content-type: application/json' \
            --data "${PAYLOAD}"  ${VERBOSE}
        echo
        ((j = j + 1 ))
    done
    echo 
done
echo

echo "Verify device are created"
curl -u "${APIKEY}:${APITOKEN}" -k --url https://${ORG}.internetofthings.ibmcloud.com/api/v0002/bulk/devices ${VERBOSE}
echo
echo

