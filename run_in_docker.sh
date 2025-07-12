#!/bin/bash
set -e
apt-get update -qq
apt-get install -y -qq netcat-openbsd
rm -rf build
./run_tests.sh