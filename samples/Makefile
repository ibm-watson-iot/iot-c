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

current_dir := $(shell pwd)
parent_dir := $(shell dirname ${current_dir})

ifndef blddir
    blddir = ${parent_dir}/build/samples
endif

ifndef samplesrcdir
    samplesrcdir = ${parent_dir}/samples
endif

ifndef srcdir
    srcdir = ${parent_dir}/src
endif

ifndef pahomqttdir
  pahomqttdir = $(parent_dir)/paho.mqtt.c
endif

CC ?= gcc
RM = rm -f

INCDIRS = -I $(srcdir) -I $(pahomqttdir)/src
LIBDIRS = -L $(parent_dir)/build -L $(pahomqttdir)/build/output
CFLAGS = $(INCDIRS) -fPIC -Wall -Wextra -O2 -g
LDFLAGS = $(LIBDIRS) -lpaho-mqtt3as -liotp-utils -liotp-as-client -liotp-as-device -liotp-as-gateway -liotp-as-application

SAMPLE_SRCS = $(wildcard *.c)
SAMPLES = $(patsubst %.c, $(blddir)/%, $(SAMPLE_SRCS))

.PHONY: all clean ${SAMPLES}

all: mkdir build

mkdir:
	@mkdir -p $(blddir)/samples

build: ${SAMPLES}

${SAMPLES}: ${blddir}/%: ${samplesrcdir}/%.c
	$(CC) -o $@ $< ${CFLAGS} ${LDFLAGS}

clean:
	-${RM} -r $(blddir)

print-% : ; @echo $* = $($*)

