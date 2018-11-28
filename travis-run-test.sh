#!/bin/bash

cd iot-c

CURDIR=`pwd`
export CURDIR

OSTYPE=`uname -s`
export OSTYPE

IOTPLIBDIR=${CURDIR}/build
export IOTPLIBDIR

PAHOLIBDIR=${CURDIR}/paho.mqtt.c/build/output
export PAHOLIBDIR

TESTBINDIR=${CURDIR}/build/test
export TESTBINDIR

echo "OSTYPE: ${OSTYPE}"

if [ "${OSTYPE}" == "Darwin" ]
then
    DYLD_LIBRARY_PATH=${PAHOLIBDIR}:${IOTPLIBDIR} ${TESTBINDIR}/validateConfig_tests_as
else
    LD_LIBRARY_PATH=${PAHOLIBDIR}:${IOTPLIBDIR} ${TESTBINDIR}/validateConfig_tests_as
fi

