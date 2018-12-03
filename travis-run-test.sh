#!/bin/bash

set -e

cd iot-c

CURDIR=`pwd`
export CURDIR

OSTYPE=`uname -s`
export OSTYPE

echo "Run tests"

echo "Get Device Data from platform"

TESTORG="${orgid}"
export TESTORG

echo "OrgID: ${TESTORG}"
curl --request GET \
     -u "${apikey}:${token}" -k -v --url https://${orgid}.internetofthings.ibmcloud.com/api/v0002/bulk/devices


# echo "Run tests"
# make -C test run_tests

