#!/bin/bash
#
#*******************************************************************************
#  Copyright (c) 2019 IBM Corp.
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
#     Ranjan Dasgupta - initial drop
#
#*******************************************************************************/
#
# Script for code coverage report
#

ostype=$1
topdir=$2
paholibdir=$3
coverdir=$4
coverlogfile=$5

# code coverage environment variables
export GCOV_PREFIX=$coverdir
export GCOV_PREFIX_STRIP=`echo $topdir | awk '{print gsub(/\//, "")}'`
export COMPREPORT=YES

LIBLIST=(iotp-device-coverage-as-lib iotp-gateway-coverage-as-lib iotp-application-coverage-as-lib iotp-managed-device-coverage-as-lib iotp-managed-gateway-coverage-as-lib)

# LIBLIST=(iotp-device-coverage-as-lib)

TESTLIST=(device_tests_coverage gateway_tests_coverage application_tests_coverage managedDevice_tests_coverage managedGateway_tests_coverage)

# TESTLIST=(device_tests_coverage)

if [ "${ostype}" == "Darwin" ]; then
    export DYLD_LIBRARY_PATH=$paholibdir:$coverdir
else
    export LD_LIBRARY_PATH=$paholibdir:$coverdir
fi

mkdir -p ${coverdir}/reports

count=0

slog="${coverdir}/reports/coverage.log"
rm -f $slog
touch $slog

(

while [ "x${LIBLIST[count]}" != "x" ]
do
    libname=${LIBLIST[count]}
    tname=${TESTLIST[count]}

    cd ${topdir}

    echo
    echo "Build coverage lib: ${libname}"
    if [ "${libname}" == "skip" ] ; then
        echo "Required library is available"
    else
        rm -f ${coverdir}/*.gcno
        cd $topdir
        make ${libname}
    fi

    echo
    echo "Coverage Test: ${tname}"
    make -C test $tname

    testname=`echo $tname | cut -d"_" -f1`
    testCFile=$testname_tests.c

    cd ${coverdir}

    echo
    echo "Run test ${tname}"

    if [ "${ostype}" == "Darwin" ]; then
        DYLD_LIBRARY_PATH=$paholibdir:$coverdir; ./$tname
    else
        LD_LIBRARY_PATH=$paholibdir:$coverdir; ./$tname
    fi

    echo
    echo "Coverage report: ${tname}"
    mv build/coverage/*.gcda .

    tlog="${coverdir}/reports/${testname}.log"
    llog="${coverdir}/reports/${testname}_lib.log"

    rm -f $tlog
    touch $tlog
    rm -f $llog
    touch $llog

    echo >> $slog
    echo "Test ${testname}_tests coverage summary" >> $slog
    echo >> $slog
    printf "%s\t\t%s\t%s\t%s\n" "FileName" "Lines" "LinesExecuted" "BranchesExecuted" >> $slog
    echo "-----------------------------------------------------------------" >> $slog

    echo "gcov -a -b -c -u ${coverdir}/${testname}_tests.c" 
    gcov -a -b -c -u ${coverdir}/${testname}_tests.c | tee -a ${tlog}

    # write summary
    nolines=`grep "Lines " ${tlog} | cut -d" " -f4`
    lExec=`grep "Lines executed" ${tlog} | cut -d" " -f2 | cut -d":" -f2`
    bExec=`grep "Branches executed" ${tlog} | cut -d" " -f2 | cut -d":" -f2`

    ccount=`echo "${testname}_tests.c" | wc -c`
    if [ $ccount -gt 16 ]; then 
        printf "%s\t%s\t%s\t\t%s\n" "${testname}_tests.c" "$nolines" "$lExec" "$bExec" >> $slog
    else
        printf "%s\t\t%s\t%s\t\t%s\n" "${testname}_tests.c" "$nolines" "$lExec" "$bExec" >> $slog
    fi

    echo
    echo

    rm -f test_utils.gcno 
    rm -f ${testname}_tests.gcno

    if [ "${COMPREPORT}" == "YES" ]; then

        for gcno in `ls *.gcno`
        do
            fname=`echo $gcno | cut -d"." -f1`
            echo "Process ${fname}"
    
            echo "gcov -a -b -c -u ${fname}.c" 
            gcov -a -b -c -u ${fname}.c | tee -a ${llog}

            # write summary
            nolines=`grep "Lines " ${llog} | cut -d" " -f4`
            lExec=`grep "Lines executed" ${llog} | cut -d" " -f2 | cut -d":" -f2`
            bExec=`grep "Branches executed" ${llog} | cut -d" " -f2 | cut -d":" -f2`
    
            if [ "${lExec}" != "0.00%" ] ; then
                ccount=`echo "${fname}.c" | wc -c`
                if [ $ccount -gt  16 ]; then 
                    printf "%s\t%s\t%s\t\t%s\n" "${fname}.c" "$nolines" "$lExec" "$bExec" >> $slog
                else
                    printf "%s\t\t%s\t%s\t\t%s\n" "${fname}.c" "$nolines" "$lExec" "$bExec" >> $slog
                fi
            fi
    
            rm -f ${llog}
            touch ${llog}
    
        done

    fi

    count=$(( $count + 1 ))

done

) | tee -a $coverlogfile

