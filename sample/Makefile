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
#     Ranjan Dasgupta - initial drop of WIoTP iot-c client samples
#
#*******************************************************************************/

SHELL = /bin/sh
.PHONY: clean, mkdir


ifeq ($(OS),Windows_NT)
        OSTYPE ?= $(OS)
        MACHINETYPE ?= $(PROCESSOR_ARCHITECTURE)
else
        OSTYPE ?= $(shell uname -s)
        MACHINETYPE ?= $(shell uname -m)
endif # OS

ifeq ($(OSTYPE),linux)
        OSTYPE = Linux
endif

current_dir := $(shell pwd)
parent_dir := $(shell dirname $(current_dir))

ifndef blddir
    blddir = $(parent_dir)/build/sample
endif

ifndef samplesrcdir
    samplesrcdir = $(parent_dir)/sample
endif

ifndef srcdir
    srcdir = $(parent_dir)/src
endif

ifndef pahomqttdir
  pahomqttdir = $(parent_dir)/paho.mqtt.c
endif

CC ?= gcc
RM = rm -f

INCDIRS = -I $(srcdir) -I $(pahomqttdir)/src
LIBDIRS = -L $(parent_dir)/build -L $(pahomqttdir)/build/output
CFLAGS = $(INCDIRS) -fPIC -Wall -Wextra -O2 -g
IOTPLIBS = -lpaho-mqtt3as -liotp-as-device -liotp-as-gateway -liotp-as-application
LDFLAGS_DEV = ${START_GROUP} -lpthread -lssl -lcrypto ${END_GROUP} $(LIBDIRS) -lpaho-mqtt3as -liotp-as-device
LDFLAGS_GWY = ${START_GROUP} -lpthread -lssl -lcrypto ${END_GROUP} $(LIBDIRS) -lpaho-mqtt3as -liotp-as-gateway
LDFLAGS_APP = ${START_GROUP} -lpthread -lssl -lcrypto ${END_GROUP} $(LIBDIRS) -lpaho-mqtt3as -liotp-as-application
LDFLAGS_MANAGED_DEV = ${START_GROUP} -lpthread -lssl -lcrypto ${END_GROUP} $(LIBDIRS) -lpaho-mqtt3as -liotp-as-managedDevice

ifeq ($(OSTYPE),Linux)

START_GROUP = -Wl,--start-group
END_GROUP = -Wl,--end-group
EXTRA_LIB = -ldl
FLAGS_EXES +=

else ifeq ($(OSTYPE),Darwin)

START_GROUP =
END_GROUP =
EXTRA_LIB = -ldl
FLAGS_EXES += -L /usr/local/opt/openssl/lib

endif


DEV_SAMPLE_SRCS = $(wildcard device*.c)
DEV_SAMPLES = $(patsubst %.c, $(blddir)/%, $(DEV_SAMPLE_SRCS))

GWY_SAMPLE_SRCS = $(wildcard gateway*.c)
GWY_SAMPLES = $(patsubst %.c, $(blddir)/%, $(GWY_SAMPLE_SRCS))

APP_SAMPLE_SRCS = $(wildcard application*.c)
APP_SAMPLES = $(patsubst %.c, $(blddir)/%, $(APP_SAMPLE_SRCS))

MANAGED_DEV_SAMPLE_SRCS = $(wildcard managedDevice*.c)
MANAGED_DEV_SAMPLES = $(patsubst %.c, $(blddir)/%, $(MANAGED_DEV_SAMPLE_SRCS))


.PHONY: all clean $(DEV_SAMPLES) $(GWY_SAMPLES) $(APP_SAMPLES) $(MANAGED_DEV_SAMPLES)

all: mkdir build

mkdir:
	@mkdir -p $(blddir)

build: $(DEV_SAMPLES) $(GWY_SAMPLES) $(APP_SAMPLES) $(MANAGED_DEV_SAMPLES)


$(DEV_SAMPLES): $(blddir)/%: $(DEV_SAMPLE_SRCS)
	$(CC) -g -o $@ $< $(INCDIRS) $(LDFLAGS_DEV) $(FLAGS_EXES)

$(GWY_SAMPLES): $(blddir)/%: $(GWY_SAMPLE_SRCS)
	$(CC) -g -o $@ $< $(INCDIRS) $(LDFLAGS_GWY) $(FLAGS_EXES)

$(APP_SAMPLES): $(blddir)/%: $(APP_SAMPLE_SRCS)
	$(CC) -g -o $@ $< $(INCDIRS) $(LDFLAGS_APP) $(FLAGS_EXES)

$(MANAGED_DEV_SAMPLES): $(blddir)/%: $(MANAGED_DEV_SAMPLE_SRCS)
	$(CC) -g -o $@ $< $(INCDIRS) $(LDFLAGS_MANAGED_DEV) $(FLAGS_EXES)


clean:
	-$(RM) -r $(blddir)

print-% : ; @echo $* = $($*)

