#!/bin/bash

west flash --runner jlink --build-dir build/rp2040
west flash --esp-device /dev/ttyUSB2 --build-dir build/esp32
