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

#if defined(__cplusplus)
 extern "C" {
#endif

#if !defined(TEST_UTILS_H_)
#define TEST_UTILS_H_

#define DLLImport extern
#define DLLExport __attribute__ ((visibility ("default")))

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <libgen.h>

#define TEST_COUNT(x) (sizeof(x)/sizeof(x[0]))
#define TEST_ASSERT(desc, val, fmt...) testAssert(__FILE__, __LINE__, desc, val, fmt)

DLLExport void testStart(const char * desc, int count);
DLLExport void testAssert(const char *fname, int lineno, char *desc, int val, const char *fmt, ...);
DLLExport void testEnd(const char *desc, int count);


#endif 


#if defined(__cplusplus)
 }
#endif

