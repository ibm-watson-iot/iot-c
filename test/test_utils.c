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
 *
 * Contrinutors:
 *    Ranjan Dasgupta         - Initial drop
 *
 *******************************************************************************/

#include "test_utils.h"

int total = 0;
int failures = 0;

void testStart(const char *desc, int count)
{
    printf("\nStart TEST: %s\n", desc);
    printf("Number of TestSuites: %d\n\n", count);
    total = 0;
    failures = 0;
}

void testAssert(const char *fname, int lineno, char *desc, int val, const char *fmt, ...)
{
    va_list args;
    char buf[4096];

    va_start(args, fmt);
    vsnprintf(buf, 4096, fmt, args);
    va_end(args);

    ++total;
    if (!val) {
        ++failures;
        printf("%2d | Failed | %s | %d | %s | %s\n", total, basename((char *)fname), lineno, desc, buf);
    } else {
        printf("%2d | Passed | %s | %d | %s | %s\n", total, basename((char *)fname), lineno, desc, buf);
    }
    fflush(stdout);
}

void testEnd(void)
{
    printf("\nTest Status: Total:%d  Failures:%d\n\n", total, failures);
}

