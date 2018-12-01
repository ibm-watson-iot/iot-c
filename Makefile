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
#
# IBM Watson IoT platform MQTT Async client library
# - The APIs in this library is used by WIoTP client libraries
#   - Device, Gateway, Application, Managed (device and gateway)
#
CLIENT_AS_C = iotp_async.c
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
DEVICE_AS_C = $(UTILS_C) $(CLIENT_AS_C) iotp_device.c
DEVICE_AS_O = $(DEVICE_AS_C:.c=.o)
DEVICE_AS_SRCS = $(addprefix $(srcdir)/,$(DEVICE_AS_C))
DEVICE_AS_OBJS = $(addprefix $(blddir)/,$(DEVICE_AS_O))
DEVICE_AS_LIB_NAME = iotp-as-device
DEVICE_AS_LIB_TARGET = $(blddir)/lib$(DEVICE_AS_LIB_NAME).so.$(VERSION)

# Gateway Library
GATEWAY_AS_C = $(UTILS_C) $(CLIENT_AS_C) iotp_gateway.c
GATEWAY_AS_O = $(GATEWAY_AS_C:.c=.o)
GATEWAY_AS_SRCS = $(addprefix $(srcdir)/,$(GATEWAY_AS_C))
GATEWAY_AS_OBJS = $(addprefix $(blddir)/,$(GATEWAY_AS_O))
GATEWAY_AS_LIB_NAME = iotp-as-gateway
GATEWAY_AS_LIB_TARGET = $(blddir)/lib$(GATEWAY_AS_LIB_NAME).so.$(VERSION)

# Application Library
APPLICATION_AS_C = $(UTILS_C) $(CLIENT_AS_C) iotp_application.c
APPLICATION_AS_O = $(APPLICATION_AS_C:.c=.o)
APPLICATION_AS_SRCS = $(addprefix $(srcdir)/,$(APPLICATION_AS_C))
APPLICATION_AS_OBJS = $(addprefix $(blddir)/,$(APPLICATION_AS_O))
APPLICATION_AS_LIB_NAME = iotp-as-application
APPLICATION_AS_LIB_TARGET = $(blddir)/lib$(APPLICATION_AS_LIB_NAME).so.$(VERSION)

# IoTP Library (optional)
# For device applications that may include APIs exposed by device or gateway library
IOTP_AS_C = $(UTILS_C) $(CLIENT_AS_C) iotp_device.c iotp_gateway.c iotp_application.c
IOTP_AS_O = $(IOTP_AS_C:.c=.o)
IOTP_AS_SRCS = $(addprefix $(srcdir)/,$(IOTP_AS_C))
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
	echo OSTYPE is $(OSTYPE)


# Paho MQTT C library
paho-mqtt: mkdir paho-mqtt-download paho-mqtt-build

# IoTP asynchronous client libraries 
iotp-version: $(blddir)/iotp_version.h
iotp-device-as-lib: paho-mqtt iotp-version $(DEVICE_AS_LIB_TARGET)
iotp-gateway-as-lib: paho-mqtt iotp-version $(GATEWAY_AS_LIB_TARGET)
iotp-application-as-lib: paho-mqtt iotp-version $(APPLICATION_AS_LIB_TARGET)
iotp-as-libs: iotp-device-as-lib iotp-gateway-as-lib iotp-application-as-lib

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

$(blddir)/iotp_version.h: $(srcdir)/iotp_version.h.in
	$(SED_COMMAND) $< > $@

$(blddir)/%.o: $(srcdir)/%.c
	$(CC) $(CCFLAGS_SO) -c $< -o $@

$(DEVICE_AS_LIB_TARGET): $(DEVICE_AS_SRCS) $(blddir)/iotp_version.h
	$(CC) $(CCFLAGS_SO) -o $@ $(DEVICE_AS_SRCS) $(LDFLAGS_AS)
	-ln -s lib$(DEVICE_AS_LIB_NAME).so.$(VERSION) $(blddir)/lib$(DEVICE_AS_LIB_NAME).so.$(MAJOR_VERSION)
	-ln -s lib$(DEVICE_AS_LIB_NAME).so.$(MAJOR_VERSION) $(blddir)/lib$(DEVICE_AS_LIB_NAME).so

$(GATEWAY_AS_LIB_TARGET): $(GATEWAY_AS_SRCS) $(blddir)/iotp_version.h
	$(CC) $(CCFLAGS_SO) -o $@ $(GATEWAY_AS_SRCS) $(LDFLAGS_AS)
	-ln -s lib$(GATEWAY_AS_LIB_NAME).so.$(VERSION) $(blddir)/lib$(GATEWAY_AS_LIB_NAME).so.$(MAJOR_VERSION)
	-ln -s lib$(GATEWAY_AS_LIB_NAME).so.$(MAJOR_VERSION) $(blddir)/lib$(GATEWAY_AS_LIB_NAME).so

$(APPLICATION_AS_LIB_TARGET): $(APPLICATION_AS_SRCS) $(blddir)/iotp_version.h
	$(CC) $(CCFLAGS_SO) -o $@ $(APPLICATION_AS_SRCS) $(LDFLAGS_AS)
	-ln -s lib$(APPLICATION_AS_LIB_NAME).so.$(VERSION) $(blddir)/lib$(APPLICATION_AS_LIB_NAME).so.$(MAJOR_VERSION)
	-ln -s lib$(APPLICATION_AS_LIB_NAME).so.$(MAJOR_VERSION) $(blddir)/lib$(APPLICATION_AS_LIB_NAME).so

$(IOTP_AS_LIB_TARGET): $(IOTP_AS_SRCS) $(blddir)/iotp_version.h
	$(CC) $(CCFLAGS_SO) -o $@ $(IOTP_AS_SRCS) $(LDFLAGS_AS)
	-ln -s lib$(IOTP_AS_LIB_NAME).so.$(VERSION) $(blddir)/lib$(IOTP_AS_LIB_NAME).so.$(MAJOR_VERSION)
	-ln -s lib$(IOTP_AS_LIB_NAME).so.$(MAJOR_VERSION) $(blddir)/lib$(IOTP_AS_LIB_NAME).so

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
