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
#     Ranjan Dasgupta - initial drop of iot-c test
#
#*******************************************************************************/
#
# Test setup
#
# Creates and deletes set of device types and devices in WIoTP service.
#

ACTION=$1
if [ $# -ne 1 ]
then
    echo "USAGE: $0 setup|cleanup|testconnection"
    exit 1
fi

# Script action - valid actions are create and delete
ACTION=$1

# Organization ID, key and token are from encrypted environment variables in travis.
#
echo "WIOTP_IDENTITY_ORGID=${WIOTP_IDENTITY_ORGID}"
echo "WIOTP_AUTH_KEY=${WIOTP_AUTH_KEY}"
echo "WIOTP_AUTH_TOKEN=${WIOTP_AUTH_TOKEN}"

DEVTOKEN="${WIOTP_AUTH_TOKEN}"
ORG=${WIOTP_AUTH_KEY:2:6}

# Specify number of device types.
# Script will add device types: iot_test_devType1, iot_test_devType2, ...
NODEVTYPE=1

# Specify number of devices of each types
# Script will add devices: iot_test_dev1, iot_test_dev2, ...
NODEV=2

echo
echo "Run tests using WiOTP instance ${ORG}"
echo

# Set for verbose output
VERBOSE="-v"

#
# Test connection to platform
#
if [ "${ACTION}" == "testconnection" ]
then
    echo
    echo "Test WIoTP connectivity"
    curl -u "${WIOTP_AUTH_KEY}:${WIOTP_AUTH_TOKEN}" -ks -o /dev/null -w "%{http_code}" --url https://${ORG}.internetofthings.ibmcloud.com/api/v0002/bulk/devices
    echo
    exit 0
fi


#
# Create action
#
if [ "${ACTION}" == "setup" ]
then
 
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
        curl -k -u "${WIOTP_AUTH_KEY}:${WIOTP_AUTH_TOKEN}" --request POST \
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
            curl -k -u "${WIOTP_AUTH_KEY}:${WIOTP_AUTH_TOKEN}" --request POST \
                --url https://${ORG}.internetofthings.ibmcloud.com/api/v0002/device/types/${devType}/devices \
                --header 'content-type: application/json' \
                --data "${PAYLOAD}"  ${VERBOSE}
            echo
            ((j = j + 1 ))
        done
        echo 
    
        echo "Verify devices are created"
        curl -u "${WIOTP_AUTH_KEY}:${WIOTP_AUTH_TOKEN}" -k ${VERBOST} \
             --url https://${ORG}.internetofthings.ibmcloud.com/api/v0002/bulk/devices?typeId=${devType}
        echo
        echo

    done
    echo


    #
    # Create Gateway Types
    #
    i=1
    while [ $i -le $NODEVTYPE ]
    do
        devType="iotc_test_gwType${i}"
        echo "Create gateway type: ${devType}"
        PAYLOAD="{\"id\":\"${devType}\",\"classId\":\"Gateway\",\"deviceInfo\":{},\"metadata\":{}}"
        # echo ${PAYLOAD}
        curl -k -u "${WIOTP_AUTH_KEY}:${WIOTP_AUTH_TOKEN}" --request POST \
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
            dev="iotc_test_gw${j}"
            echo "Create gateway: ${dev}"
            PAYLOAD="{\"deviceId\":\"${dev}\",\"authToken\":\"${DEVTOKEN}\",\"deviceInfo\":{},\"location\":{},\"metadata\":{}}"
            curl -k -u "${WIOTP_AUTH_KEY}:${WIOTP_AUTH_TOKEN}" --request POST \
                --url https://${ORG}.internetofthings.ibmcloud.com/api/v0002/device/types/${devType}/devices \
                --header 'content-type: application/json' \
                --data "${PAYLOAD}"  ${VERBOSE}
            echo
            ((j = j + 1 ))
        done
        echo

        echo "Verify gateways are created"
        curl -u "${WIOTP_AUTH_KEY}:${WIOTP_AUTH_TOKEN}" -k ${VERBOST} \
             --url https://${ORG}.internetofthings.ibmcloud.com/api/v0002/bulk/devices?typeId=${devType}
        echo
        echo

    done
    echo

    exit 0

fi


#
# Delete action
#
if [ "${ACTION}" == "cleanup" ]
then

    #
    # Delete device types and devices
    i=1
    while [ $i -le $NODEVTYPE ]
    do
        devType="iotc_test_devType${i}"
    
        #
        # Delete Devices
        #
        j=1
        while [ $j -le $NODEV ]
        do
            dev="iotc_test_dev${j}"
            echo "Delete device: ${dev}"
            curl --request DELETE -u "${WIOTP_AUTH_KEY}:${WIOTP_AUTH_TOKEN}" -k  ${VERBOSE} \
                --url https://${ORG}.internetofthings.ibmcloud.com/api/v0002/device/types/${devType}/devices/${dev}

            echo
            ((j = j + 1 ))
        done
        echo 

        echo "Verify device are deleted"
        curl -u "${WIOTP_AUTH_KEY}:${WIOTP_AUTH_TOKEN}" -k ${VERBOST} \
             --url https://${ORG}.internetofthings.ibmcloud.com/api/v0002/bulk/devices?typeId=${devType}
        echo
        echo

    
        #
        # Delete Device Type
        #
        echo "Delete device type: ${devType}"
        curl --request DELETE -u "${WIOTP_AUTH_KEY}:${WIOTP_AUTH_TOKEN}" -k \
             --url https://${ORG}.internetofthings.ibmcloud.com/api/v0002/device/types/${devType}  ${VERBOSE}
        echo
        echo
        ((i = i + 1 ))
    done
    echo

    #
    # Delete gateway types and devices
    i=1
    while [ $i -le $NODEVTYPE ]
    do
        devType="iotc_test_gwType${i}"

        #
        # Delete Gateways
        #
        j=1
        while [ $j -le $NODEV ]
        do
            dev="iotc_test_gw${j}"
            echo "Delete gateway: ${dev}"
            curl --request DELETE -u "${WIOTP_AUTH_KEY}:${WIOTP_AUTH_TOKEN}" -k  ${VERBOSE} \
                --url https://${ORG}.internetofthings.ibmcloud.com/api/v0002/device/types/${devType}/devices/${dev}

            echo
            ((j = j + 1 ))
        done
        echo

        echo "Verify gateways are deleted"
        curl -u "${WIOTP_AUTH_KEY}:${WIOTP_AUTH_TOKEN}" -k ${VERBOST} \
             --url https://${ORG}.internetofthings.ibmcloud.com/api/v0002/bulk/devices?typeId=${devType}
        echo
        echo

        #
        # Delete gateway Types
        #
        echo "Delete gateway type: ${devType}"
        curl --request DELETE -u "${WIOTP_AUTH_KEY}:${WIOTP_AUTH_TOKEN}" -k \
             --url https://${ORG}.internetofthings.ibmcloud.com/api/v0002/device/types/${devType}  ${VERBOSE}
        echo
        echo
        ((i = i + 1 ))
    done
    echo
    
    exit 0

fi

echo
echo "ERROR: Unsupported action ${ACTION} is specified"
echo "USAGE: $0 create|delete"
echo
exit 1
