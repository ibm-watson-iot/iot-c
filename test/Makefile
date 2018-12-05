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
    blddir = $(parent_dir)/build/test
endif

ifndef testdir
    testdir = $(parent_dir)/test
endif

ifndef srcdir
    srcdir = $(parent_dir)/src
endif

ifndef iotplibdir
  iotplibdir = $(parent_dir)/build
endif

ifndef paholibdir
  paholibdir = $(parent_dir)/paho.mqtt.c/build/output
endif

ifndef pahomqttdir
  pahomqttdir = $(parent_dir)/paho.mqtt.c
endif

CC ?= gcc
RM = rm -f

INCDIRS = -I $(srcdir) -I $(pahomqttdir)/src
LIBDIRS = -L $(parent_dir)/build -L $(pahomqttdir)/build/output
CFLAGS = $(INCDIRS) -fPIC -Wall -Wextra -O2 -g
LDFLAGS_DEV = ${START_GROUP} -lpthread -lssl -lcrypto ${END_GROUP} $(LIBDIRS) -lpaho-mqtt3as -liotp-as-device
LDFLAGS_GWY = ${START_GROUP} -lpthread -lssl -lcrypto ${END_GROUP} $(LIBDIRS) -lpaho-mqtt3as -liotp-as-gateway
LDFLAGS_APP = ${START_GROUP} -lpthread -lssl -lcrypto ${END_GROUP} $(LIBDIRS) -lpaho-mqtt3as -liotp-as-application

ifeq ($(OSTYPE),Linux)

START_GROUP = -Wl,--start-group
END_GROUP = -Wl,--end-group
EXTRA_LIB = -ldl

define run-test
    @echo
    @echo ==== Run Test: $(notdir $(1))
    @echo ==== Time: $(shell date +%T)
    @echo ---- Use MQTT Paho asynchronous library
    LD_LIBRARY_PATH=$(paholibdir):$(iotplibdir) $(blddir)/$(1)
endef

else ifeq ($(OSTYPE),Darwin)

START_GROUP =
END_GROUP =
EXTRA_LIB = -ldl
FLAGS_EXES += -L /usr/local/opt/openssl/lib

define run-test
    @echo
    @echo ==== Run Test: $(notdir $(1))
    @echo ==== Time: $(shell date +%T)
    @echo ---- Use MQTT Paho asynchronous library
    DYLD_LIBRARY_PATH=$(paholibdir):$(iotplibdir) $(blddir)/$(1)
endef

endif

TEST_UTIL_SRCS = test_utils.c

VAL_CONFIG_TEST_SRCS = validateConfig_tests.c
VAL_CONFIG_TEST = $(patsubst %.c, $(blddir)/%, $(VAL_CONFIG_TEST_SRCS))

VAL_DEVICE_TEST_SRCS = validateDevice_tests.c
VAL_DEVICE_TEST = $(patsubst %.c, $(blddir)/%, $(VAL_DEVICE_TEST_SRCS))

SYS_DEVICE_TEST_SRCS = systemDevice_tests.c
SYS_DEVICE_TEST = $(patsubst %.c, $(blddir)/%, $(SYS_DEVICE_TEST_SRCS))


TEST_RUN = validateConfig_tests validateDevice_tests systemDevice_tests


.PHONY: all clean $(VAT_CONFIG_TEST) $(VAL_DEVICE_TEST) $(SYS_DEVICE_TEST)

all: mkdir build

mkdir:
	@mkdir -p $(blddir)

build: $(VAL_CONFIG_TEST) $(VAL_DEVICE_TEST) $(SYS_DEVICE_TEST)

run_tests: $(TEST_RUN)


$(VAL_CONFIG_TEST): $(TEST_UTIL_SRCS) $(VAL_CONFIG_TEST_SRCS)
	$(CC) -o $@ $(TEST_UTIL_SRCS) $(VAL_CONFIG_TEST_SRCS) $(INCDIRS) $(LDFLAGS_DEV) $(FLAGS_EXES)

$(VAL_DEVICE_TEST): $(TEST_UTIL_SRCS) $(VAL_DEVICE_TEST_SRCS)
	$(CC) -o $@ $(TEST_UTIL_SRCS) $(VAL_DEVICE_TEST_SRCS) $(INCDIRS) $(LDFLAGS_DEV) $(FLAGS_EXES)

$(SYS_DEVICE_TEST): $(TEST_UTIL_SRCS) $(SYS_DEVICE_TEST_SRCS)
	$(CC) -o $@ $(TEST_UTIL_SRCS) $(SYS_DEVICE_TEST_SRCS) $(INCDIRS) $(LDFLAGS_DEV) $(FLAGS_EXES)

$(TEST_RUN):
	$(call run-test,$@)

clean:
	-$(RM) -r $(blddir)

print-% : ; @echo $* = $($*)

