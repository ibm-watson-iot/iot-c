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
#     Ranjan Dasgupta - initial drop of WIoTP iot-c client
#
#*******************************************************************************/
#
# This script setups up test environment and execute unit and functional tests
#

# set -e

cd iot-c

CURDIR=`pwd`
export CURDIR

OSTYPE=`uname -s`
export OSTYPE

echo "Running tests on ${OSTYPE}"

#
# Test Setup - create device types and devices in the platform test organization
#
echo "===================================================================="
echo "    Setup tests: create device types and devices in WIoTP service"
echo "===================================================================="
chmod +x ./test/test.sh
./test/test.sh setup
echo
echo

#
# Run tests
#
rm -f test.log
echo "===================================================================="
echo "    Run test suites"
echo "===================================================================="
if [ "${OSTYPE}" == "Darwin" ]
then
    echo "By pass test"
else
    ( make run-tests ) | tee -a test.log
fi
echo

echo "===================================================================="
echo "    Test execution summary"
echo "===================================================================="
echo
echo "Failed tests:"
echo "-------------"
grep "TEST-" test.log | grep "Failed"
echo
echo "Test Summaries:"
echo "---------------"
grep "TEST-Summary" test.log
echo
echo


#
# Test Cleanup - delete device types and devices in the platform test organization
#
echo "===================================================================="
echo "    Setup tests: delete device types and devices in WIoTP service"
echo "===================================================================="
./test/test.sh cleanup
echo
echo



