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
#     Ranjan Dasgupta - initial API implementation and documentation
# 
#*******************************************************************************/

SHELL = /bin/sh
.PHONY: clean, mkdir, install, uninstall, buildreference, builddocs

TOP ?= $(shell pwd)

ifndef release.version
  release.version = 0.1
endif

# determine current platform
BUILD_TYPE ?= debug

ifeq ($(OS),Windows_NT)
	OSTYPE ?= $(OS)
	MACHINETYPE ?= $(PROCESSOR_ARCHITECTURE)
else
	OSTYPE ?= $(shell uname -s)
	MACHINETYPE ?= $(shell uname -m)
	build.level = $(shell date)
endif # OS

ifeq ($(OSTYPE),linux)
	OSTYPE = Linux
endif

CC = gcc
LDCONFIG ?= /sbin/ldconfig
ifndef INSTALL
INSTALL = install
endif
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA =  $(INSTALL) -m 644
DOXYGEN_COMMAND = doxygen

# code coverage environment variables
export GCOV_PREFIX = $(TOP)/build/test/coverage/bin
export GCOV_PREFIX_STRIP = $(shell pwd | awk '{print gsub(/\//, "")}')

#
# Project directories
#
ifndef srcdir
  srcdir = src/wiotp/sdk
endif

ifndef blddir
  blddir = build
endif

ifndef coverdir
  coverdir = $(TOP)/build/coverage
endif

ifndef docdir
  docdir = doc
endif

ifndef docs
  docs = docs
endif

ifndef prefix
	prefix = /usr/local
endif

ifndef exec_prefix
	exec_prefix = $(prefix)
endif

ifndef iotplibdir
  iotplibdir = $(TOP)/build
endif

ifndef paholibdir
  paholibdir = $(TOP)/paho.mqtt.c/build/output
endif

ifndef coverlogfile
  coverlogfile = $(TOP)/temp/cunit_tests_coverage.log
endif

bindir = $(exec_prefix)/bin
includedir = $(prefix)/include
libdir = $(exec_prefix)/lib
iotplibdir = $(abspath $(blddir))

#
# Paho MQTT directories and libraries
# 
ifndef pahomqttdir 
  pahomqttdir = $(TOP)/paho.mqtt.c
endif
paholibdir = $(abspath $(pahomqttdir)/build/output)
PAHO_MQTT_CS_LIB_NAME = paho-mqtt3cs
PAHO_MQTT_AS_LIB_NAME = paho-mqtt3as


#
# WIoTP client library version
#
MAJOR_VERSION = 1
MINOR_VERSION = 0
VERSION = $(MAJOR_VERSION).$(MINOR_VERSION)
# Command options to create version.h 
SED_COMMAND = sed \
    -e "s/@IOTP_CLIENT_VERSION@/$(release.version)/g" \
    -e "s/@BUILD_TIMESTAMP@/$(build.level)/g"


#
# IBM Watson IoT platform utility library
# Includes configuration APIs, Logging APIs, Error codes and utility functions
#
UTILS_C = iotp_utils.c iotp_config.c
UTILS_H = iotp_utils.h iotp_config.h iotp_rc.h
#
# IBM Watson IoT platform MQTT Async client library
# - The APIs in this library is used by WIoTP client libraries
#   - Device, Gateway, Application, Managed (device and gateway)
#
CLIENT_AS_C = iotp_async.c
CLIENT_AS_H = iotp_internal.h
# 
# WIoTP Async client libraries, for:
# - Device
# - Gateway
# - Application
# - Managed Device
# - Managed Gateway
# These librarirs depends on iotp-utils and iotp-mqttas libraries
#
# Device Library
#
DEVICE_AS_C = $(UTILS_C) $(CLIENT_AS_C) iotp_device.c
DEVICE_AS_H = $(UTILS_H) $(CLIENT_AS_H) iotp_device.h
DEVICE_AS_O = $(DEVICE_AS_C:.c=.o)
DEVICE_AS_SRCS = $(addprefix $(srcdir)/,$(DEVICE_AS_C))
DEVICE_AS_HEADERS = $(addprefix $(srcdir)/,$(DEVICE_AS_H))
DEVICE_AS_OBJS = $(addprefix $(blddir)/,$(DEVICE_AS_O))
DEVICE_AS_LIB_NAME = iotp-as-device
DEVICE_AS_LIB_TARGET = $(blddir)/lib$(DEVICE_AS_LIB_NAME).so.$(VERSION)
DEVICE_AS_COVERAGE_OBJS = $(addprefix $(coverdir)/,$(DEVICE_AS_O))
DEVICE_AS_COVERAGE_LIB_TARGET = $(coverdir)/lib$(DEVICE_AS_LIB_NAME).so.$(VERSION)

# Gateway Library
GATEWAY_AS_C = $(UTILS_C) $(CLIENT_AS_C) iotp_gateway.c
GATEWAY_AS_H = $(UTILS_H) $(CLIENT_AS_H) iotp_gateway.h
GATEWAY_AS_O = $(GATEWAY_AS_C:.c=.o)
GATEWAY_AS_SRCS = $(addprefix $(srcdir)/,$(GATEWAY_AS_C))
GATEWAY_AS_HEADERS = $(addprefix $(srcdir)/,$(GATEWAY_AS_H))
GATEWAY_AS_OBJS = $(addprefix $(blddir)/,$(GATEWAY_AS_O))
GATEWAY_AS_LIB_NAME = iotp-as-gateway
GATEWAY_AS_LIB_TARGET = $(blddir)/lib$(GATEWAY_AS_LIB_NAME).so.$(VERSION)
GATEWAY_AS_COVERAGE_OBJS = $(addprefix $(coverdir)/,$(GATEWAY_AS_O))
GATEWAY_AS_COVERAGE_LIB_TARGET = $(coverdir)/lib$(GATEWAY_AS_LIB_NAME).so.$(VERSION)

# Application Library
APPLICATION_AS_C = $(UTILS_C) $(CLIENT_AS_C) iotp_application.c
APPLICATION_AS_H = $(UTILS_H) $(CLIENT_AS_H) iotp_application.h
APPLICATION_AS_O = $(APPLICATION_AS_C:.c=.o)
APPLICATION_AS_SRCS = $(addprefix $(srcdir)/,$(APPLICATION_AS_C))
APPLICATION_AS_HEADERS = $(addprefix $(srcdir)/,$(APPLICATION_AS_H))
APPLICATION_AS_OBJS = $(addprefix $(blddir)/,$(APPLICATION_AS_O))
APPLICATION_AS_LIB_NAME = iotp-as-application
APPLICATION_AS_LIB_TARGET = $(blddir)/lib$(APPLICATION_AS_LIB_NAME).so.$(VERSION)
APPLICATION_AS_COVERAGE_OBJS = $(addprefix $(coverdir)/,$(APPLICATION_AS_O))
APPLICATION_AS_COVERAGE_LIB_TARGET = $(coverdir)/lib$(APPLICATION_AS_LIB_NAME).so.$(VERSION)

# Managed Device Library
MANAGED_DEVICE_AS_C = $(UTILS_C) $(CLIENT_AS_C) iotp_managedDevice.c
MANAGED_DEVICE_AS_H = $(UTILS_H) $(CLIENT_AS_H) iotp_managedDevice.h
MANAGED_DEVICE_AS_O = $(MANAGED_DEVICE_AS_C:.c=.o)
MANAGED_DEVICE_AS_SRCS = $(addprefix $(srcdir)/,$(MANAGED_DEVICE_AS_C))
MANAGED_DEVICE_AS_HEADERS = $(addprefix $(srcdir)/,$(MANAGED_DEVICE_AS_H))
MANAGED_DEVICE_AS_OBJS = $(addprefix $(blddir)/,$(MANAGED_DEVICE_AS_O))
MANAGED_DEVICE_AS_LIB_NAME = iotp-as-managedDevice
MANAGED_DEVICE_AS_LIB_TARGET = $(blddir)/lib$(MANAGED_DEVICE_AS_LIB_NAME).so.$(VERSION)
MANAGED_DEVICE_AS_COVERAGE_OBJS = $(addprefix $(coverdir)/,$(MANAGED_DEVICE_AS_O))
MANAGED_DEVICE_AS_COVERAGE_LIB_TARGET = $(coverdir)/lib$(MANAGED_DEVICE_AS_LIB_NAME).so.$(VERSION)

# Managed Gateway Library
MANAGED_GATEWAY_AS_C = $(UTILS_C) $(CLIENT_AS_C) iotp_managedGateway.c
MANAGED_GATEWAY_AS_H = $(UTILS_H) $(CLIENT_AS_H) iotp_managedGateway.h
MANAGED_GATEWAY_AS_O = $(MANAGED_GATEWAY_AS_C:.c=.o)
MANAGED_GATEWAY_AS_SRCS = $(addprefix $(srcdir)/,$(MANAGED_GATEWAY_AS_C))
MANAGED_GATEWAY_AS_HEADERS = $(addprefix $(srcdir)/,$(MANAGED_GATEWAY_AS_H))
MANAGED_GATEWAY_AS_OBJS = $(addprefix $(blddir)/,$(MANAGED_GATEWAY_AS_O))
MANAGED_GATEWAY_AS_LIB_NAME = iotp-as-managedGateway
MANAGED_GATEWAY_AS_LIB_TARGET = $(blddir)/lib$(MANAGED_GATEWAY_AS_LIB_NAME).so.$(VERSION)
MANAGED_GATEWAY_AS_COVERAGE_OBJS = $(addprefix $(coverdir)/,$(MANAGED_GATEWAY_AS_O))
MANAGED_GATEWAY_AS_COVERAGE_LIB_TARGET = $(coverdir)/lib$(MANAGED_GATEWAY_AS_LIB_NAME).so.$(VERSION)

# IoTP Library (optional)
# For device applications that may include APIs exposed by application, device, gateway or managed library 
IOTP_AS_C = $(UTILS_C) $(CLIENT_AS_C) iotp_device.c iotp_gateway.c iotp_application.c iotp_managed_device.c
IOTP_AS_H = $(UTILS_H) $(CLIENT_AS_H) iotp_device.h iotp_gateway.h iotp_application.h iotp_managed_device.h
IOTP_AS_O = $(IOTP_AS_C:.c=.o)
IOTP_AS_SRCS = $(addprefix $(srcdir)/,$(IOTP_AS_C))
IOTP_AS_HEADERS = $(addprefix $(srcdir)/,$(IOTP_AS_H))
IOTP_AS_OBJS = $(addprefix $(blddir)/,$(IOTP_AS_O))
IOTP_AS_LIB_NAME = iotp-as
IOTP_AS_LIB_TARGET = $(blddir)/lib$(IOTP_AS_LIB_NAME).so.$(VERSION)

#
# Compiler flags
#
INCDIRS = -I $(TOP)/$(srcdir) -I $(TOP)/$(blddir) -I $(pahomqttdir)/src
LIBDIRS = -L $(TOP)/$(blddir) -L $(pahomqttdir)/build/output
EXELIBS = $(START_GROUP) -lpthread -lssl -lcrypto $(END_GROUP)
LDLIBS  = $(START_GROUP) -lpthread -lssl -lcrypto -ldl $(END_GROUP)
DEFINES = -DOPENSSL -DOPENSSL_LOAD_CONF

CCFLAGS_SO = $(CFLAGS) -g -fPIC -Os -Wall -fvisibility=hidden $(INCDIRS) $(DEFINES)
LDFLAGS_AS = $(LDFLAGS) $(DEFINES) -shared $(LDLIBS) $(LIBDIRS) -l$(PAHO_MQTT_AS_LIB_NAME)
FLAGS_EXES = $(LDFLAGS) $(INCDIRS) $(EXELIBS) $(LIBDIRS)

# Coverage build
COVERAGE_AS_C = $(UTILS_C) $(CLIENT_AS_C) iotp_device.c iotp_gateway.c iotp_application.c iotp_managedDevice.c iotp_managedGateway.c
COVERAGE_AS_SRCS = $(addprefix $(srcdir)/,$(COVERAGE_AS_C))
COVERAGE_AS_O = $(COVERAGE_AS_C:.c=.o)
COVERAGE_AS_OBJS = $(addprefix $(coverdir)/bin/,$(COVERAGE_AS_O))
COVERAGE_CCFLAGS_SO = $(CCFLAGS_SO) -fprofile-arcs -ftest-coverage

#
# OS Specifics Flags, helper functions
#
ifeq ($(OSTYPE),Linux)

START_GROUP = -Wl,--start-group
END_GROUP = -Wl,--end-group
LDFLAGS_AS_BEGIN = -Wl,-soname,lib
LDFLAGS_AS_END = .so.$(MAJOR_VERSION) -Wl,-no-whole-archive
LDCONFIG = echo
SED_I = sed -i

else ifeq ($(OSTYPE),Darwin)

START_GROUP =
END_GROUP =
LIBDIRS += -L /usr/local/opt/openssl/lib
CCFLAGS_SO += -Wno-deprecated-declarations -DOSX -I /usr/local/opt/openssl/include -I $(pahomqttdir)/src
LDFLAGS_AS_BEGIN = -Wl,-install_name,lib
LDFLAGS_AS_END = .so.$(MAJOR_VERSION) $(LIBDIRS)
FLAGS_EXES += -L /usr/local/opt/openssl/lib -L $(TOP)/$(blddir)
LDCONFIG = echo
SED_I = sed -i bak

endif

#
# By default build IoTP asynchronus client libraries
#
all: build

build: | mkdir iotp-as-libs

clean:
	rm -rf $(blddir)/*

mkdir:
	-mkdir -p $(blddir)
	-mkdir -p $(blddir)/test
	echo OSTYPE is $(OSTYPE)


# Paho MQTT C library
paho-mqtt: mkdir paho-mqtt-download paho-mqtt-build

# IoTP asynchronous client libraries 
iotp-version: $(blddir)/iotp_version.h
iotp-device-as-lib: paho-mqtt iotp-version $(DEVICE_AS_LIB_TARGET)
iotp-gateway-as-lib: paho-mqtt iotp-version $(GATEWAY_AS_LIB_TARGET)
iotp-application-as-lib: paho-mqtt iotp-version $(APPLICATION_AS_LIB_TARGET)
iotp-managedDevice-as-lib: paho-mqtt iotp-version $(MANAGED_DEVICE_AS_LIB_TARGET)
iotp-managedGateway-as-lib: paho-mqtt iotp-version $(MANAGED_GATEWAY_AS_LIB_TARGET)
iotp-as-libs: iotp-device-as-lib iotp-gateway-as-lib iotp-application-as-lib iotp-managedDevice-as-lib iotp-managedGateway-as-lib

# Client libraries for code coverage
iotp-device-coverage-as-lib: paho-mqtt iotp-version $(DEVICE_AS_COVERAGE_LIB_TARGET)
iotp-gateway-coverage-as-lib: paho-mqtt iotp-version $(GATEWAY_AS_COVERAGE_LIB_TARGET)
iotp-application-coverage-as-lib: paho-mqtt iotp-version $(APPLICATION_AS_COVERAGE_LIB_TARGET)
iotp-managed-device-coverage-as-lib: paho-mqtt iotp-version $(MANAGED_DEVICE_AS_COVERAGE_LIB_TARGET)
iotp-managed-gateway-coverage-as-lib: paho-mqtt iotp-version $(MANAGED_GATEWAY_AS_COVERAGE_LIB_TARGET)

iotp-coverage-as-libs: iotp-device-coverage-as-lib iotp-gateway-coverage-as-lib iotp-application-coverage-as-lib iotp-managed-device-coverage-as-lib iotp-managed-gateway-coverage-as-lib

iotp-coverage-as-build:


# IoTP asynchrous client library with device, gateway, and application APIs
iotp-as-lib: paho-mqtt iotp-version $(IOTP_AS_LIB_TARGET)


paho-mqtt-download:
	echo "Downloading paho mqtt c source and setup for build"
	@if test ! -d paho.mqtt.c; \
	then \
	mkdir -p download; \
	cd download; \
	curl -LJO https://github.com/eclipse/paho.mqtt.c/archive/v1.3.0.tar.gz; \
	cd .. ; \
	tar xzf download/paho.mqtt.c-1.3.0.tar.gz; \
	mv paho.mqtt.c-1.3.0 paho.mqtt.c; \
	$(SED_I) 's/-DOPENSSL /-DOPENSSL -DOPENSSL_LOAD_CONF /g' paho.mqtt.c/Makefile; \
	fi

paho-mqtt-build:
	echo "Build paho mqtt c"
	make -C paho.mqtt.c

tests:
	make -C test

run-tests:
	make -C test run_tests

coverage:
	@echo "GCOV_PREFIX = $(GCOV_PREFIX)"
	@echo "GCOV_PREFIX_STRIP = $(GCOV_PREFIX_STRIP)"
	make -C test init-log-coverage
	$(TOP)/coverage.sh $(OSTYPE) $(TOP) $(paholibdir) $(coverdir) $(coverlogfile)

get-test-summary:
	make -C test get-summary

samples:
	make -C sample

$(blddir)/iotp_version.h: $(srcdir)/iotp_version.h.in
	$(SED_COMMAND) $< > $@

$(blddir)/%.o: $(srcdir)/%.c
	$(CC) $(CCFLAGS_SO) -c $< -o $@

$(COVERAGE_AS_OBJS): $(COVERAGE_AS_SRCS)
	$(CC) $(COVERAGE_CCFLAGS_SO) -c $< -o $@

$(DEVICE_AS_LIB_TARGET): $(DEVICE_AS_SRCS) $(DEVICE_AS_HEADERS) $(blddir)/iotp_version.h
	$(CC) $(CCFLAGS_SO) -o $@ $(DEVICE_AS_SRCS) $(LDFLAGS_AS)
	-ln -s lib$(DEVICE_AS_LIB_NAME).so.$(VERSION) $(blddir)/lib$(DEVICE_AS_LIB_NAME).so.$(MAJOR_VERSION)
	-ln -s lib$(DEVICE_AS_LIB_NAME).so.$(MAJOR_VERSION) $(blddir)/lib$(DEVICE_AS_LIB_NAME).so

$(GATEWAY_AS_LIB_TARGET): $(GATEWAY_AS_SRCS) $(GATEWAY_AS_HEADERS) $(blddir)/iotp_version.h
	$(CC) $(CCFLAGS_SO) -o $@ $(GATEWAY_AS_SRCS) $(LDFLAGS_AS)
	-ln -s lib$(GATEWAY_AS_LIB_NAME).so.$(VERSION) $(blddir)/lib$(GATEWAY_AS_LIB_NAME).so.$(MAJOR_VERSION)
	-ln -s lib$(GATEWAY_AS_LIB_NAME).so.$(MAJOR_VERSION) $(blddir)/lib$(GATEWAY_AS_LIB_NAME).so

$(APPLICATION_AS_LIB_TARGET): $(APPLICATION_AS_SRCS) $(APPLICATION_AS_HEADERS) $(blddir)/iotp_version.h
	$(CC) $(CCFLAGS_SO) -o $@ $(APPLICATION_AS_SRCS) $(LDFLAGS_AS)
	-ln -s lib$(APPLICATION_AS_LIB_NAME).so.$(VERSION) $(blddir)/lib$(APPLICATION_AS_LIB_NAME).so.$(MAJOR_VERSION)
	-ln -s lib$(APPLICATION_AS_LIB_NAME).so.$(MAJOR_VERSION) $(blddir)/lib$(APPLICATION_AS_LIB_NAME).so

$(MANAGED_DEVICE_AS_LIB_TARGET): $(MANAGED_DEVICE_AS_SRCS) $(MANAGED_DEVICE_AS_HEADERS) $(blddir)/iotp_version.h
	$(CC) $(CCFLAGS_SO) -o $@ $(MANAGED_DEVICE_AS_SRCS) $(LDFLAGS_AS)
	-ln -s lib$(MANAGED_DEVICE_AS_LIB_NAME).so.$(VERSION) $(blddir)/lib$(MANAGED_DEVICE_AS_LIB_NAME).so.$(MAJOR_VERSION)
	-ln -s lib$(MANAGED_DEVICE_AS_LIB_NAME).so.$(MAJOR_VERSION) $(blddir)/lib$(MANAGED_DEVICE_AS_LIB_NAME).so

$(MANAGED_GATEWAY_AS_LIB_TARGET): $(MANAGED_GATEWAY_AS_SRCS) $(MANAGED_GATEWAY_AS_HEADERS) $(blddir)/iotp_version.h
	$(CC) $(CCFLAGS_SO) -o $@ $(MANAGED_GATEWAY_AS_SRCS) $(LDFLAGS_AS)
	-ln -s lib$(MANAGED_GATEWAY_AS_LIB_NAME).so.$(VERSION) $(blddir)/lib$(MANAGED_GATEWAY_AS_LIB_NAME).so.$(MAJOR_VERSION)
	-ln -s lib$(MANAGED_GATEWAY_AS_LIB_NAME).so.$(MAJOR_VERSION) $(blddir)/lib$(MANAGED_GATEWAY_AS_LIB_NAME).so

$(IOTP_AS_LIB_TARGET): $(IOTP_AS_SRCS) $(IOTP_AS_HEADERS) $(blddir)/iotp_version.h
	$(CC) $(CCFLAGS_SO) -o $@ $(IOTP_AS_SRCS) $(LDFLAGS_AS)
	-ln -s lib$(IOTP_AS_LIB_NAME).so.$(VERSION) $(blddir)/lib$(IOTP_AS_LIB_NAME).so.$(MAJOR_VERSION)
	-ln -s lib$(IOTP_AS_LIB_NAME).so.$(MAJOR_VERSION) $(blddir)/lib$(IOTP_AS_LIB_NAME).so

# Coverage build
$(DEVICE_AS_COVERAGE_LIB_TARGET): $(DEVICE_AS_SRCS) $(DEVICE_AS_HEADERS) $(blddir)/iotp_version.h
	$(CC) $(COVERAGE_CCFLAGS_SO) -o $@ $(DEVICE_AS_SRCS) $(LDFLAGS_AS)
	-mv *.gcno $(coverdir)/.
	-ln -s $(coverdir)/lib$(DEVICE_AS_LIB_NAME).so.$(VERSION) $(coverdir)/lib$(DEVICE_AS_LIB_NAME).so.$(MAJOR_VERSION)
	-ln -s $(coverdir)/lib$(DEVICE_AS_LIB_NAME).so.$(MAJOR_VERSION) $(coverdir)/lib$(DEVICE_AS_LIB_NAME).so

$(GATEWAY_AS_COVERAGE_LIB_TARGET): $(GATEWAY_AS_SRCS) $(GATEWAY_AS_HEADERS) $(blddir)/iotp_version.h
	$(CC) $(COVERAGE_CCFLAGS_SO) -o $@ $(GATEWAY_AS_SRCS) $(LDFLAGS_AS)
	-mv *.gcno $(coverdir)/.
	-ln -s $(coverdir)/lib$(GATEWAY_AS_LIB_NAME).so.$(VERSION) $(coverdir)/lib$(GATEWAY_AS_LIB_NAME).so.$(MAJOR_VERSION)
	-ln -s $(coverdir)/lib$(GATEWAY_AS_LIB_NAME).so.$(MAJOR_VERSION) $(coverdir)/lib$(GATEWAY_AS_LIB_NAME).so

$(APPLICATION_AS_COVERAGE_LIB_TARGET): $(APPLICATION_AS_SRCS) $(APPLICATION_AS_HEADERS) $(blddir)/iotp_version.h
	$(CC) $(COVERAGE_CCFLAGS_SO) -o $@ $(APPLICATION_AS_SRCS) $(LDFLAGS_AS)
	-mv *.gcno $(coverdir)/.
	-ln -s $(coverdir)/lib$(APPLICATION_AS_LIB_NAME).so.$(VERSION) $(coverdir)/lib$(APPLICATION_AS_LIB_NAME).so.$(MAJOR_VERSION)
	-ln -s $(coverdir)/lib$(APPLICATION_AS_LIB_NAME).so.$(MAJOR_VERSION) $(coverdir)/lib$(APPLICATION_AS_LIB_NAME).so

$(MANAGED_DEVICE_AS_COVERAGE_LIB_TARGET): $(MANAGED_DEVICE_AS_SRCS) $(MANAGED_DEVICE_AS_HEADERS) $(blddir)/iotp_version.h
	$(CC) $(COVERAGE_CCFLAGS_SO) -o $@ $(MANAGED_DEVICE_AS_SRCS) $(LDFLAGS_AS)
	-mv *.gcno $(coverdir)/.
	-ln -s $(coverdir)/lib$(MANAGED_DEVICE_AS_LIB_NAME).so.$(VERSION) $(coverdir)/lib$(MANAGED_DEVICE_AS_LIB_NAME).so.$(MAJOR_VERSION)
	-ln -s $(coverdir)/lib$(MANAGED_DEVICE_AS_LIB_NAME).so.$(MAJOR_VERSION) $(coverdir)/lib$(MANAGED_DEVICE_AS_LIB_NAME).so

$(MANAGED_GATEWAY_AS_COVERAGE_LIB_TARGET): $(MANAGED_GATEWAY_AS_SRCS) $(MANAGED_GATEWAY_AS_HEADERS) $(blddir)/iotp_version.h
	$(CC) $(COVERAGE_CCFLAGS_SO) -o $@ $(MANAGED_GATEWAY_AS_SRCS) $(LDFLAGS_AS)
	-mv *.gcno $(coverdir)/.
	-ln -s $(coverdir)/lib$(MANAGED_GATEWAY_AS_LIB_NAME).so.$(VERSION) $(coverdir)/lib$(MANAGED_GATEWAY_AS_LIB_NAME).so.$(MAJOR_VERSION)
	-ln -s $(coverdir)/lib$(MANAGED_GATEWAY_AS_LIB_NAME).so.$(MAJOR_VERSION) $(coverdir)/lib$(MANAGED_GATEWAY_AS_LIB_NAME).so

strip_options:
	$(eval INSTALL_OPTS := -s)

install-strip: build strip_options install

# install: install-as install-cs
install: install-as

install-as: build
	mkdir -p $(DESTDIR)$(PREFIX)$(includedir)
	$(INSTALL_DATA) $(INSTALL_OPTS) $(IOTPLIB_AS_TARGET) $(DESTDIR)$(libdir)
	-ln -s lib$(IOTPLIB_AS).so.$(MAJOR_VERSION) $(DESTDIR)$(libdir)/lib$(IOTPLIB_AS).so
	@if test ! -f $(DESTDIR)$(libdir)/lib$(IOTPLIB_AS).so.$(MAJOR_VERSION); then ln -s lib$(IOTPLIB_AS).so.$(VERSION) $(DESTDIR)$(libdir)/lib$(IOTPLIB_AS).so.$(MAJOR_VERSION); fi
	$(INSTALL_DATA) $(srcdir)/iotp_mqttclient.h $(DESTDIR)$(includedir)
	$(LDCONFIG) $(DESTDIR)$(libdir)
	
uninstall:
	- rm $(DESTDIR)$(libdir)/$(IOTPLIB_AS) 
	- rm $(DESTDIR)$(libdir)/lib$(IOTPLIB_AS).so
	- rm $(DESTDIR)$(libdir)/lib$(IOTPLIB_AS).so.$(MAJOR_VERSION)
	- rm $(DESTDIR)$(includedir)/iotp_mqttclient.h
	$(LDCONFIG) $(DESTDIR)$(libdir)

REGEX_DOXYGEN := \
    's;@PROJECT_SOURCE_DIR@/src/wiotp/sdk/\?;;' \
    's;@PROJECT_SOURCE_DIR@;../../..;' \
    's;@CMAKE_CURRENT_BINARY_DIR@;../build;'

SED_DOXYGEN := $(foreach sed_exp,$(REGEX_DOXYGEN),-e $(sed_exp))
define process_doxygen
	cd $(srcdir); sed $(SED_DOXYGEN) $(TOP)/doc/doxygen/$(1).in > $(TOP)/doc/$(1)
	cd $(srcdir); $(DOXYGEN_COMMAND) $(TOP)/doc/$(1)
endef

buildreference:
	-mkdir -p $(docdir)/IoTP_C_Client_Docs
	-mkdir -p $(docs)/reference
	$(call process_doxygen,DoxyfileIOTPClientAPI)
	-cp $(docdir)/doxygen/navtree.css $(docs)/reference/.

builddocs:
	-mkdir -p $(docs)
	cd mkdocs; mkdocs build -d ../docs

# Print a variable
print-%  : ; @echo $* = $($*)

