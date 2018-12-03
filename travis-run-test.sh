#!/bin/bash

set -e

cd iot-c

CURDIR=`pwd`
export CURDIR

OSTYPE=`uname -s`
export OSTYPE

echo "Run tests"
echo "ORGID: $orgid"
make -C test run_tests

