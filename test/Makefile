#*******************************************************************************
#  Copyright (c) 2018-2019 IBM Corp.
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
#     Ranjan Dasgupta - initial drop of iot-c tests
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

ifndef coverdir
    coverdir = $(parent_dir)/build/coverage
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

ifndef logfile
  logfile = $(parent_dir)/temp/cunit_tests.log
endif

ifndef coverlogfile
  coverlogfile = $(parent_dir)/temp/cunit_tests_coverage.log
endif

CC = gcc
RM = rm -f

INCDIRS = -I $(srcdir)/wiotp/sdk -I $(pahomqttdir)/src
LIBDIRS = -L $(parent_dir)/build -L $(pahomqttdir)/build/output
CFLAGS = $(INCDIRS) -fPIC -Wall -Wextra -g

CFLAGS_COVERAGE = $(CFLAGS) -fprofile-arcs -ftest-coverage
LIBDIRS_COVERAGE = -L $(coverdir) -L $(pahomqttdir)/build/output

LDFLAGS_DEV = ${START_GROUP} -lpthread -lssl -lcrypto ${END_GROUP} $(LIBDIRS) -lpaho-mqtt3as -liotp-as-device
LDFLAGS_GWY = ${START_GROUP} -lpthread -lssl -lcrypto ${END_GROUP} $(LIBDIRS) -lpaho-mqtt3as -liotp-as-gateway
LDFLAGS_APP = ${START_GROUP} -lpthread -lssl -lcrypto ${END_GROUP} $(LIBDIRS) -lpaho-mqtt3as -liotp-as-application
LDFLAGS_MDV = ${START_GROUP} -lpthread -lssl -lcrypto ${END_GROUP} $(LIBDIRS) -lpaho-mqtt3as -liotp-as-managedDevice
LDFLAGS_MGW = ${START_GROUP} -lpthread -lssl -lcrypto ${END_GROUP} $(LIBDIRS) -lpaho-mqtt3as -liotp-as-managedGateway

LDFLAGS_DEV_COVERAGE = ${START_GROUP} -lpthread -lssl -lcrypto ${END_GROUP} $(LIBDIRS_COVERAGE) -lpaho-mqtt3as -liotp-as-device
LDFLAGS_GWY_COVERAGE = ${START_GROUP} -lpthread -lssl -lcrypto ${END_GROUP} $(LIBDIRS_COVERAGE) -lpaho-mqtt3as -liotp-as-gateway
LDFLAGS_APP_COVERAGE = ${START_GROUP} -lpthread -lssl -lcrypto ${END_GROUP} $(LIBDIRS_COVERAGE) -lpaho-mqtt3as -liotp-as-application
LDFLAGS_MDV_COVERAGE = ${START_GROUP} -lpthread -lssl -lcrypto ${END_GROUP} $(LIBDIRS_COVERAGE) -lpaho-mqtt3as -liotp-as-managedDevice
LDFLAGS_MGW_COVERAGE = ${START_GROUP} -lpthread -lssl -lcrypto ${END_GROUP} $(LIBDIRS_COVERAGE) -lpaho-mqtt3as -liotp-as-managedGateway

# Defines to run unit tests and tests for code coverage
ifeq ($(OSTYPE),Linux)

START_GROUP = -Wl,--start-group
END_GROUP = -Wl,--end-group
EXTRA_LIB = -ldl

define run-test
    @echo
    @echo ==== Run Test: $(notdir $(1))
    @echo ==== Time: $(shell date +%T)
    @echo ---- Use MQTT Paho asynchronous library
    LD_LIBRARY_PATH=$(paholibdir):$(iotplibdir) $(blddir)/$(1) | tee -a $(logfile)
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
	DYLD_LIBRARY_PATH=$(paholibdir):$(iotplibdir) $(blddir)/$(1) | tee -a $(logfile)
endef

endif

TEST_UTIL_SRCS = test_utils.c

CONFIG_TEST_SRCS = config_tests.c
CONFIG_TEST = $(patsubst %.c, $(blddir)/%, $(CONFIG_TEST_SRCS))
CONFIG_TEST_COVERAGE = $(patsubst %.c, $(coverdir)/%_coverage, $(CONFIG_TEST_SRCS))

DEVICE_TEST_SRCS = device_tests.c
DEVICE_TEST = $(patsubst %.c, $(blddir)/%, $(DEVICE_TEST_SRCS))
DEVICE_TEST_COVERAGE = $(patsubst %.c, $(coverdir)/%_coverage, $(DEVICE_TEST_SRCS))

GATEWAY_TEST_SRCS = gateway_tests.c
GATEWAY_TEST = $(patsubst %.c, $(blddir)/%, $(GATEWAY_TEST_SRCS))
GATEWAY_TEST_COVERAGE = $(patsubst %.c, $(coverdir)/%_coverage, $(GATEWAY_TEST_SRCS))

APPLICATION_TEST_SRCS = application_tests.c
APPLICATION_TEST = $(patsubst %.c, $(blddir)/%, $(APPLICATION_TEST_SRCS))
APPLICATION_TEST_COVERAGE = $(patsubst %.c, $(coverdir)/%_coverage, $(APPLICATION_TEST_SRCS))

MANAGED_DEVICE_TEST_SRCS = managedDevice_tests.c
MANAGED_DEVICE_TEST = $(patsubst %.c, $(blddir)/%, $(MANAGED_DEVICE_TEST_SRCS))
MANAGED_DEVICE_TEST_COVERAGE = $(patsubst %.c, $(coverdir)/%_coverage, $(MANAGED_DEVICE_TEST_SRCS))

MANAGED_GATEWAY_TEST_SRCS = managedGateway_tests.c
MANAGED_GATEWAY_TEST = $(patsubst %.c, $(blddir)/%, $(MANAGED_GATEWAY_TEST_SRCS))
MANAGED_GATEWAY_TEST_COVERAGE = $(patsubst %.c, $(coverdir)/%_coverage, $(MANAGED_GATEWAY_TEST_SRCS))


TEST_RUN = config_tests device_tests gateway_tests application_tests managedDevice_tests managedGateway_tests


.PHONY: all clean

all: mkdir build

mkdir:
	@mkdir -p $(blddir)
	@mkdir -p $(coverdir)
	@cp $(srcdir)/wiotp/sdk/* $(coverdir)/.
	@cp $(testdir)/* $(coverdir)/.
 

build: $(CONFIG_TEST) $(DEVICE_TEST) $(GATEWAY_TEST) $(APPLICATION_TEST) $(MANAGED_DEVICE_TEST) $(MANAGED_GATEWAY_TEST)

$(CONFIG_TEST): $(TEST_UTIL_SRCS) $(CONFIG_TEST_SRCS)
	$(CC) $(CFLAGS) -o $@ $(TEST_UTIL_SRCS) $(CONFIG_TEST_SRCS) $(INCDIRS) $(LDFLAGS_DEV) $(FLAGS_EXES)

$(DEVICE_TEST): $(TEST_UTIL_SRCS) $(DEVICE_TEST_SRCS)
	$(CC) $(CFLAGS) -o $@ $(TEST_UTIL_SRCS) $(DEVICE_TEST_SRCS) $(INCDIRS) $(LDFLAGS_DEV) $(FLAGS_EXES)

$(GATEWAY_TEST): $(TEST_UTIL_SRCS) $(GATEWAY_TEST_SRCS)
	$(CC) $(CFLAGS) -o $@ $(TEST_UTIL_SRCS) $(GATEWAY_TEST_SRCS) $(INCDIRS) $(LDFLAGS_GWY) $(FLAGS_EXES)

$(APPLICATION_TEST): $(TEST_UTIL_SRCS) $(APPLICATION_TEST_SRCS)
	$(CC) $(CFLAGS) -o $@ $(TEST_UTIL_SRCS) $(APPLICATION_TEST_SRCS) $(INCDIRS) $(LDFLAGS_APP) $(FLAGS_EXES)

$(MANAGED_DEVICE_TEST): $(TEST_UTIL_SRCS) $(MANAGED_DEVICE_TEST_SRCS)
	$(CC) $(CFLAGS) -o $@ $(TEST_UTIL_SRCS) $(MANAGED_DEVICE_TEST_SRCS) $(INCDIRS) $(LDFLAGS_MDV) $(FLAGS_EXES)

$(MANAGED_GATEWAY_TEST): $(TEST_UTIL_SRCS) $(MANAGED_GATEWAY_TEST_SRCS)
	$(CC) $(CFLAGS) -o $@ $(TEST_UTIL_SRCS) $(MANAGED_GATEWAY_TEST_SRCS) $(INCDIRS) $(LDFLAGS_MGW) $(FLAGS_EXES)


#
# Coverage tests build rules:
#

config_tests_coverage: $(TEST_UTIL_SRCS) $(CONFIG_TEST_SRCS)
	cd $(coverdir); $(CC) $(CFLAGS_COVERAGE) -o $(coverdir)/$@ $(TEST_UTIL_SRCS) $(CONFIG_TEST_SRCS) $(INCDIRS) $(LDFLAGS_DEV_COVERAGE) $(FLAGS_EXES)

device_tests_coverage: $(TEST_UTIL_SRCS) $(DEVICE_TEST_SRCS)
	cd $(coverdir); $(CC) $(CFLAGS_COVERAGE) -o $@ $(TEST_UTIL_SRCS) $(DEVICE_TEST_SRCS) $(INCDIRS) $(LDFLAGS_DEV_COVERAGE) $(FLAGS_EXES)

gateway_tests_coverage: $(TEST_UTIL_SRCS) $(GATEWAY_TEST_SRCS)
	cd $(coverdir); $(CC) $(CFLAGS_COVERAGE) -o $@ $(TEST_UTIL_SRCS) $(GATEWAY_TEST_SRCS) $(INCDIRS) $(LDFLAGS_GWY_COVERAGE) $(FLAGS_EXES)

application_tests_coverage: $(TEST_UTIL_SRCS) $(APPLICATION_TEST_SRCS)
	cd $(coverdir); $(CC) $(CFLAGS_COVERAGE) -o $@ $(TEST_UTIL_SRCS) $(APPLICATION_TEST_SRCS) $(INCDIRS) $(LDFLAGS_APP_COVERAGE) $(FLAGS_EXES)

managedDevice_tests_coverage: $(TEST_UTIL_SRCS) $(MANAGED_DEVICE_TEST_SRCS)
	cd $(coverdir); $(CC) $(CFLAGS_COVERAGE) -o $@ $(TEST_UTIL_SRCS) $(MANAGED_DEVICE_TEST_SRCS) $(INCDIRS) $(LDFLAGS_MDV_COVERAGE) $(FLAGS_EXES)

managedGateway_tests_coverage: $(TEST_UTIL_SRCS) $(MANAGED_GATEWAY_TEST_SRCS)
	cd $(coverdir); $(CC) $(CFLAGS_COVERAGE) -o $@ $(TEST_UTIL_SRCS) $(MANAGED_GATEWAY_TEST_SRCS) $(INCDIRS) $(LDFLAGS_MGW_COVERAGE) $(FLAGS_EXES)


$(TEST_RUN):
	$(call run-test,$@)


run_tests: init-log $(TEST_RUN)

init-log:
	-@mkdir -p $(parent_dir)/temp
	-$(RM) $(logfile)
	@touch $(logfile)

init-log-coverage:
	-@mkdir -p $(parent_dir)/temp
	-$(RM) $(coverlogfile)
	@touch $(coverlogfile)

get-summary:
	@echo
	@echo ==== Cunit Test Summary
	@echo ==== Time: $(shell date +%T)
	@echo
	@echo Failed tests:
	@echo ------------
	-cat $(logfile) | grep " | Failed | "
	@echo
	@echo Summary:
	@echo ------- 
	-cat $(logfile) | grep "TEST-Summary: "
	@echo

clean:
	-$(RM) -r $(blddir)
	-$(RM) -f $(logfile)

print-% : ; @echo $* = $($*)

