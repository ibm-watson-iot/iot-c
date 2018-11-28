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
#     Ranjan Dasgupta - initial API implementation and documentation
# 
#*******************************************************************************/

SHELL = /bin/sh
.PHONY: clean, mkdir, install, uninstall, html

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

CC ?= gcc
LDCONFIG ?= /sbin/ldconfig
ifndef INSTALL
INSTALL = install
endif
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA =  $(INSTALL) -m 644
DOXYGEN_COMMAND = doxygen

#
# Project directories
#
ifndef srcdir
  srcdir = src
endif

ifndef blddir
  blddir = build
endif

ifndef docdir
  docdir = doc
endif

ifndef docs
  docs = doc
endif

ifndef prefix
	prefix = /usr/local
endif

ifndef exec_prefix
	exec_prefix = $(prefix)
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
UTILS_O = $(UTILS_C:.c=.o)
UTILS_SRCS = $(addprefix $(srcdir)/,$(UTILS_C))
UTILS_OBJS = $(addprefix $(blddir)/,$(UTILS_O))
UTILS_LIB_NAME = iotp-utils
UTILS_LIB_TARGET = $(blddir)/lib$(UTILS_LIB_NAME).so.$(VERSION)

#
# IBM Watson IoT platform MQTT Async client library
# - The APIs in this library is used by WIoTP client libraries
#   - Device, Gateway, Application, Managed (device and gateway)
#
CLIENT_AS_C = iotp_async.c
CLIENT_AS_O = $(CLIENT_AS_C:.c=.o)
CLIENT_AS_SRCS = $(addprefix $(srcdir)/,$(CLIENT_AS_C))
CLIENT_AS_OBJS = $(addprefix $(blddir)/,$(CLIENT_AS_O))
CLIENT_AS_LIB_NAME = iotp-as-client
CLIENT_AS_LIB_TARGET = $(blddir)/lib$(CLIENT_AS_LIB_NAME).so.$(VERSION)

# 
# WIoTP Async client libraries, for:
# - Device
# - Gateway
# - Application
# - Managed (for device and gateway)
# These librarirs depends on iotp-utils and iotp-mqttas libraries
#
# Device Library
#
DEVICE_AS_C = iotp_device.c
DEVICE_AS_O = $(DEVICE_AS_C:.c=.o)
DEVICE_AS_SRCS = $(addprefix $(srcdir)/,$(DEVICE_AS_C))
DEVICE_AS_OBJS = $(addprefix $(blddir)/,$(DEVICE_AS_O))
DEVICE_AS_LIB_NAME = iotp-as-device
DEVICE_AS_LIB_TARGET = $(blddir)/lib$(DEVICE_AS_LIB_NAME).so.$(VERSION)

# Gateway Library
GATEWAY_AS_C = iotp_gateway.c
GATEWAY_AS_O = $(GATEWAY_AS_C:.c=.o)
GATEWAY_AS_SRCS = $(addprefix $(srcdir)/,$(GATEWAY_AS_C))
GATEWAY_AS_OBJS = $(addprefix $(blddir)/,$(GATEWAY_AS_O))
GATEWAY_AS_LIB_NAME = iotp-as-gateway
GATEWAY_AS_LIB_TARGET = $(blddir)/lib$(GATEWAY_AS_LIB_NAME).so.$(VERSION)

# Application Library
APPLICATION_AS_C = iotp_application.c
APPLICATION_AS_O = $(APPLICATION_AS_C:.c=.o)
APPLICATION_AS_SRCS = $(addprefix $(srcdir)/,$(APPLICATION_AS_C))
APPLICATION_AS_OBJS = $(addprefix $(blddir)/,$(APPLICATION_AS_O))
APPLICATION_AS_LIB_NAME = iotp-as-application
APPLICATION_AS_LIB_TARGET = $(blddir)/lib$(APPLICATION_AS_LIB_NAME).so.$(VERSION)


#
# Compiler flags
#
INCDIRS = -I $(TOP)/$(srcdir) -I $(TOP)/$(blddir) -I $(pahomqttdir)/src
LIBDIRS = -L $(TOP)/$(blddir) -L $(pahomqttdir)/build/output
EXELIBS = $(START_GROUP) -lpthread -lssl -lcrypto $(END_GROUP)
LDLIBS  = $(START_GROUP) -lpthread -lssl -lcrypto -ldl $(END_GROUP)
DEFINES = -DOPENSSL -DOPENSSL_LOAD_CONF

CCFLAGS_SO = $(CFLAGS) -g -fPIC -Os -Wall -fvisibility=hidden $(INCDIRS)
FLAGS_EXES = $(LDFLAGS) $(INCDIRS) $(EXELIBS) $(LIBDIRS) -l$(UTILS_LIB_NAME)

LDFLAGS_UTILS = $(LDFLAGS) $(DEFINES) -shared $(LDLIBS)
LDFLAGS_AS_CLIENT = $(LDFLAGS_UTILS) -l$(UTILS_LIB_NAME) -l$(PAHO_MQTT_AS_LIB_NAME)
LDFLAGS_AS = $(LDFLAGS_AS_CLIENT) -l$(CLIENT_AS_LIB_NAME)

#
# OS Specifics Flags, helper functions
#
ifeq ($(OSTYPE),Linux)

START_GROUP = -Wl,--start-group
END_GROUP = -Wl,--end-group
LDFLAGS_UTILS += -Wl,-soname,lib$(UTILS_LIB_NAME).so.$(MAJOR_VERSION) -Wl,-no-whole-archive
LDFLAGS_AS_CLIENT += -Wl,-soname,lib$(CLIENT_AS_LIB_NAME).so.$(MAJOR_VERSION) -Wl,-no-whole-archive
LDFLAGS_AS_BEGIN = -Wl,-soname,lib
LDFLAGS_AS_END = .so.$(MAJOR_VERSION) -Wl,-no-whole-archive

define run-test
    @echo
    @echo ==== Run Test: $(notdir $(1))
    @echo ==== Time: $(shell date +%T)
    @echo ---- Use MQTT Paho asynchronous library
    LD_LIBRARY_PATH=$(paholibdir):$(iotplibdir) $(blddir)/test/$(1)
endef

SED_I = sed -i

else ifeq ($(OSTYPE),Darwin)

START_GROUP =
END_GROUP =
LIBDIRS += -L /usr/local/opt/openssl/lib
CCFLAGS_SO += -Wno-deprecated-declarations -DOSX -I /usr/local/opt/openssl/include -I $(pahomqttdir)/src
LDFLAGS_CS += -Wl,-install_name,$(IOTPLIB_CS).$(MAJOR_VERSION) -L $(blddir) -L /usr/local/opt/openssl/lib -L $(pahomqttdir)/build/output
LDFLAGS_UTILS += -Wl,-install_name,lib$(UTILS_LIB_NAME).so.$(MAJOR_VERSION) $(LIBDIRS)
LDFLAGS_AS_CLIENT += -Wl,-install_name,lib$(CLIENT_AS_LIB_NAME).so.$(MAJOR_VERSION) $(LIBDIRS)
LDFLAGS_AS_BEGIN = -Wl,-install_name,lib
LDFLAGS_AS_END = .so.$(MAJOR_VERSION) $(LIBDIRS)
FLAGS_EXE += -DOSX
FLAGS_EXES += -L /usr/local/opt/openssl/lib -L $(TOP)/$(blddir)
LDCONFIG = echo

define run-test
    @echo
    @echo ==== Run Test: $(notdir $(1))
    @echo ==== Time: $(shell date +%T)
    @echo ---- Use MQTT Paho asynchronous library
    DYLD_LIBRARY_PATH=$(paholibdir):$(iotplibdir) $(blddir)/test/$(1)
endef

SED_I = sed -i bak

endif

# 
# Unit Tests
#
TEST_UTILS_C = test_utils.c
TEST_UTILS_O = $(TEST_UTILS_C:.c=.o)
TEST_UTILS_SRCS = $(addprefix test/,$(TEST_UTILS_C))
TEST_UTILS_OBJS = $(addprefix $(blddir)/test/,$(TEST_UTILS_O))
TEST_SRCS = $(wildcard test/*_tests.c)
TEST_AS_EXES = $(patsubst test/%_tests.c, $(blddir)/test/%_tests_as, $(TEST_SRCS))
TEST_CS_EXES = $(patsubst test/%_tests.c, $(blddir)/test/%_tests_cs, $(TEST_SRCS))
TEST_AS_RUN = $(patsubst test/%_tests.c, %_tests_as, $(TEST_SRCS))
TEST_CS_RUN = $(addprefix $(blddir)/test/,$(TEST_CS_EXES))

#
# By default build asynchronus library, and tests
#
# all: build test-as test-as-run test-cs test-cs-run
all: build

build: | iotp-as-libs

clean:
	rm -rf $(blddir)/*

mkdir:
	-mkdir -p $(blddir)/test
	echo OSTYPE is $(OSTYPE)

paho-mqtt: mkdir paho-mqtt-download paho-mqtt-build
iotp-version: $(blddir)/iotp_version.h
iotp-utils-lib: paho-mqtt iotp-version $(UTILS_LIB_TARGET)
iotp-client-as-lib: iotp-utils-lib $(CLIENT_AS_LIB_TARGET)

iotp-device-as-libs: iotp-client-as-lib $(DEVICE_AS_LIB_TARGET)
iotp-gateway-as-libs: iotp-client-as-lib $(GATEWAY_AS_LIB_TARGET)
iotp-application-as-libs: iotp-client-as-lib $(APPLICATION_AS_LIB_TARGET)
iotp-as-libs: iotp-device-as-libs iotp-gateway-as-libs iotp-application-as-libs

test-as: iotp-as-libs $(TEST_AS_EXES)
test-cs: iotp-client-cs-libs $(TEST_CS_EXES)

test-as-run: $(TEST_AS_RUN)
test-cs-run: $(TEST_CS_RUN)

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

$(blddir)/iotp_version.h: $(srcdir)/iotp_version.h.in
	$(SED_COMMAND) $< > $@

$(blddir)/%.o: $(srcdir)/%.c
	$(CC) $(CCFLAGS_SO) -c $< -o $@

$(UTILS_LIB_TARGET): $(UTILS_OBJS)
	$(CC) $(LDFLAGS_UTILS) -o $@ $(UTILS_OBJS)
	-ln -s lib$(UTILS_LIB_NAME).so.$(VERSION) $(blddir)/lib$(UTILS_LIB_NAME).so.$(MAJOR_VERSION)
	-ln -s lib$(UTILS_LIB_NAME).so.$(MAJOR_VERSION) $(blddir)/lib$(UTILS_LIB_NAME).so

$(CLIENT_AS_LIB_TARGET): $(CLIENT_AS_OBJS)
	$(CC) $(LDFLAGS_AS_CLIENT) -o $@ $(CLIENT_AS_OBJS)
	-ln -s lib$(CLIENT_AS_LIB_NAME).so.$(VERSION) $(blddir)/lib$(CLIENT_AS_LIB_NAME).so.$(MAJOR_VERSION)
	-ln -s lib$(CLIENT_AS_LIB_NAME).so.$(MAJOR_VERSION) $(blddir)/lib$(CLIENT_AS_LIB_NAME).so

$(DEVICE_AS_LIB_TARGET): $(DEVICE_AS_OBJS)
	$(CC) $(LDFLAGS_AS) $(LDFLAGS_AS_BEGIN)$(DEVICE_AS_LIB_NAME)$(LDFLAGS_AS_END) -o $@ $(DEVICE_AS_OBJS)
	-ln -s lib$(DEVICE_AS_LIB_NAME).so.$(VERSION) $(blddir)/lib$(DEVICE_AS_LIB_NAME).so.$(MAJOR_VERSION)
	-ln -s lib$(DEVICE_AS_LIB_NAME).so.$(MAJOR_VERSION) $(blddir)/lib$(DEVICE_AS_LIB_NAME).so

$(GATEWAY_AS_LIB_TARGET): $(GATEWAY_AS_OBJS)
	$(CC) $(LDFLAGS_AS) $(LDFLAGS_AS_BEGIN)$(GATEWAY_AS_LIB_NAME)$(LDFLAGS_AS_END) -o $@ $(GATEWAY_AS_OBJS)
	-ln -s lib$(GATEWAY_AS_LIB_NAME).so.$(VERSION) $(blddir)/lib$(GATEWAY_AS_LIB_NAME).so.$(MAJOR_VERSION)
	-ln -s lib$(GATEWAY_AS_LIB_NAME).so.$(MAJOR_VERSION) $(blddir)/lib$(GATEWAY_AS_LIB_NAME).so

$(APPLICATION_AS_LIB_TARGET): $(APPLICATION_AS_OBJS)
	$(CC) $(LDFLAGS_AS) $(LDFLAGS_AS_BEGIN)$(APPLICATION_AS_LIB_NAME)$(LDFLAGS_AS_END) -o $@ $(APPLICATION_AS_OBJS)
	-ln -s lib$(APPLICATION_AS_LIB_NAME).so.$(VERSION) $(blddir)/lib$(APPLICATION_AS_LIB_NAME).so.$(MAJOR_VERSION)
	-ln -s lib$(APPLICATION_AS_LIB_NAME).so.$(MAJOR_VERSION) $(blddir)/lib$(APPLICATION_AS_LIB_NAME).so


$(IOTPLIB_CS_TARGET): $(IOTP_CS_SRCS) $(IOTP_HEADERS) $(blddir)/iotp_version.h
	$(CC) $(CCFLAGS_SO) -o $@ $(IOTP_CS_SRCS) -DOPENSSL -DOPENSSL_LOAD_CONF $(LDFLAGS_CS) -l$(PAHO_MQTT_CS_LIB_NAME)
	-ln -s lib$(IOTPLIB_CS).so.$(VERSION)  $(blddir)/lib$(IOTPLIB_CS).so.$(MAJOR_VERSION)
	-ln -s lib$(IOTPLIB_CS).so.$(MAJOR_VERSION) $(blddir)/lib$(IOTPLIB_CS).so

$(IOTPLIB_AS_TARGET): $(IOTP_AS_SRCS) $(IOTP_HEADERS) $(blddir)/iotp_version.h
	$(CC) $(CCFLAGS_SO) -o $@ $(IOTP_AS_SRCS) -DOPENSSL -DOPENSSL_LOAD_CONF $(LDFLAGS_AS) -l$(PAHO_MQTT_AS_LIB_NAME)
	-ln -s lib$(IOTPLIB_AS).so.$(VERSION)  $(blddir)/lib$(IOTPLIB_AS).so.$(MAJOR_VERSION)
	-ln -s lib$(IOTPLIB_AS).so.$(MAJOR_VERSION) $(blddir)/lib$(IOTPLIB_AS).so

$(TEST_UTILS_OBJS): $(TEST_UTILS_SRCS)
	$(CC) -c -g -o $@ $< -I $(srcdir) -I $(pahomqttdir)/src

$(blddir)/test/%_tests.o: test/%_tests.c
	$(CC) -c -g -o $@ $< -I $(srcdir) -I $(pahomqttdir)/src

$(blddir)/test/%_tests_as: $(blddir)/test/%_tests.o $(TEST_UTILS_OBJS) iotp-as-libs
	$(CC) -g -o $@ $< $(TEST_UTILS_OBJS) -l$(CLIENT_AS_LIB_NAME) -l$(DEVICE_AS_LIB_NAME) -l$(GATEWAY_AS_LIB_NAME) -l$(APPLICATION_AS_LIB_NAME) -l$(PAHO_MQTT_AS_LIB_NAME) $(FLAGS_EXES)

%(blddir)/test/%_tests_cs: $(blddir)/test/%_tests.o $(TEST_UTILS_OBJS) $(IOTPLIB_CS_TARGET)
	$(CC) -g -o $@ $^ $(TEST_UTILS_OBJS) -l$(IOTPLIB_CS) -l$(PAHO_MQTT_CS_LIB_NAME) $(FLAGS_EXES)

$(TEST_AS_RUN):
	$(call run-test,$@)

$(TEST_CS_RUN): test-cs
	$(call run-test,$<)

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
	
install-cs: build
	mkdir -p $(DESTDIR)$(PREFIX)$(includedir)
	$(INSTALL_DATA) $(INSTALL_OPTS) $(IOTPLIB_CS_TARGET) $(DESTDIR)$(libdir)
	-ln -s lib$(IOTPLIB_CS).so.$(MAJOR_VERSION) $(DESTDIR)$(libdir)/lib$(IOTPLIB_CS).so
	@if test ! -f $(DESTDIR)$(libdir)/lib$(IOTPLIB_CS).so.$(MAJOR_VERSION); then ln -s lib$(IOTPLIB_CS).so.$(VERSION) $(DESTDIR)$(libdir)/lib$(IOTPLIB_CS).so.$(MAJOR_VERSION); fi
	$(INSTALL_DATA) $(srcdir)/iotp_mqttclient.h $(DESTDIR)$(includedir)
	$(LDCONFIG) $(DESTDIR)$(libdir)
	
uninstall:
	- rm $(DESTDIR)$(libdir)/$(IOTPLIB_CS) 
	- rm $(DESTDIR)$(libdir)/$(IOTPLIB_AS) 
	- rm $(DESTDIR)$(libdir)/lib$(IOTPLIB_CS).so
	- rm $(DESTDIR)$(libdir)/lib$(IOTPLIB_AS).so
	- rm $(DESTDIR)$(libdir)/lib$(IOTPLIB_CS).so.$(MAJOR_VERSION)
	- rm $(DESTDIR)$(libdir)/lib$(IOTPLIB_AS).so.$(MAJOR_VERSION)
	- rm $(DESTDIR)$(includedir)/iotp_mqttclient.h
	$(LDCONFIG) $(DESTDIR)$(libdir)

REGEX_DOXYGEN := \
    's;@PROJECT_SOURCE_DIR@/src/\?;;' \
    's;@PROJECT_SOURCE_DIR@;..;' \
    's;@CMAKE_CURRENT_BINARY_DIR@;../build;'

SED_DOXYGEN := $(foreach sed_exp,$(REGEX_DOXYGEN),-e $(sed_exp))
define process_doxygen
	cd $(srcdir); sed $(SED_DOXYGEN) ../doc/$(1).in > ../$(docs)/$(1)
	cd $(srcdir); $(DOXYGEN_COMMAND) ../$(docs)/$(1)
endef

html:
	-mkdir -p $(docs)/IoTP_C_Client_Docs
	$(call process_doxygen,DoxyfileIOTPClientAPI)

# Print a variable
print-%  : ; @echo $* = $($*)

