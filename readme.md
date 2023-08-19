# PL02 Firmware
This is the firwmare for the Protectli UPS. It uses zephyr RTOS running on the ESP32 and an RP2040.

## Getting Started
Before getting started, make sure you have a proper Zephyr development
environment. You can follow the official
[Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html).

```shell
west init -m git@github.com:o7-machinehum/pl02-fw.git --mr main pl02-fw
cd pl02-fw
west update
```

### Build & Run - ESP32/RP2040
The application can be built by running:

```shell
west build -b pl02_esp32 esp32-app --build-dir build/esp32
west build -b pl02_rp2040 rp2040-app --build-dir build/rp2040
```

Once you have built the application you can flash it by running:

```shell
west flash --esp-device /dev/ttyUSB0 --build-dir build/esp32
west flash --runner jlink --build-dir build/rp2040
```

```shell
# If you don't have a jlink (programmer). You can use this.
sudo picotool load build/rp2040/zephyr/zephyr.elf
```

### Build & Run - STM32
The STM32 doesn't not used zephyr, it's a libopencm3 project.
```shell
git submodule init
git submodule update
```

Install Deps
```shell
sudo pacman -S arm-none-eabi-gcc openocd # Or whatever your OS is
```

Building and flashing
```shell
cd stm32-app
make
make flash
```
