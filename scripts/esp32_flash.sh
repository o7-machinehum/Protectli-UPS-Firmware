#!/bin/bash

west build -b pl02_esp32 esp32-app --build-dir build/esp32
west flash --esp-device /dev/ttyUSB2 --build-dir build/esp32
