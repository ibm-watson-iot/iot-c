#!/bin/bash

set -e

cd iot-c
echo "Clean build environment"
make clean

echo "Download paho c client, build paho c client, build WIoTP client libraries"
make

echo "Make tests"
make -C test

echo "Make samples"
make -C samples

echo "Make doxygen docs"
make html

