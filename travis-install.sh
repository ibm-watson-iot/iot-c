#!/bin/bash

if [ "$TRAVIS_OS_NAME" == "osx" ]; then
	pwd
	brew update
        brew install doxygen
fi

pwd
git clone --single-branch -b development https://github.com/ibm-watson-iot/iot-c

