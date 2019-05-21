#!/bin/bash

set -e

cd iot-c
echo "Clean build environment"
make clean

echo "Download paho c client, build paho c client, build WIoTP client libraries"
make

echo "Make tests"
make tests

echo "Make samples"
make samples

echo "Make mkdocs and doxygen docs"
make builddocs
make buildreference


