# Linux Hardware Monitoring Tool

This project is a Linux-based hardware monitoring application written in C.
It collects real-time system metrics and displays them on an external LCD
connected via GPIO.

The project emphasizes direct interaction with Linux kernel interfaces
instead of relying on high-level monitoring libraries.

---

## Features

- Real-time CPU usage monitoring
- Memory usage tracking
- CPU temperature reading
- Direct parsing of `/proc` and `sysfs`
- LCD output via GPIO
- Developed and tested on Raspberry Pi 4B

---

## Purpose

The main purpose of this project is to understand how the Linux kernel
exposes system and hardware information to user-space programs. Instead of
using existing monitoring tools, all metrics are computed by manually parsing
kernel-provided interfaces.

---

## System Architecture

- **System Reader Module**
  - Reads CPU and memory data from `/proc`
  - Reads temperature information from `sysfs`
- **Parser Module**
  - Converts raw system files into structured data
- **Display Module**
  - Formats and outputs system metrics to the LCD
- **GPIO Interface**
  - Handles low-level LCD communication

---

## How Metrics Are Computed

CPU usage is calculated by reading cumulative CPU time values from `/proc/stat`
and computing the difference between consecutive readings. This method reflects
how standard Linux monitoring tools determine CPU utilization.

Memory usage is derived from fields in `/proc/meminfo`, allowing total and
available memory to be tracked without external dependencies.

Temperature information is obtained from kernel-exposed thermal interfaces
via `sysfs`.

---

## References

- Linux /proc File System Documentation
- Linux sysfs Documentation
- Raspberry Pi GPIO Documentation
---

## Build & Run

### Dependencies 
- GCC or Clang (C11 support required)
- CMake >= 3.16
- libgpiod (GPIO access)
- pkg-config

### On Debian-based systems:
```bash
sudo apt install cmake pkg-config libgpiod-dev
```
### On Arch Linux:
```bash
sudo pacman -S cmake pkgconf libgpiod
```
### Build
In root folder of the project
```bash
mkdir build
cd build
cmake ..
make
```
This will build:
- A static library containing the hardware monitoring and LCD logic
- An executable that links this static library and libgpiod library

### Run 
```bash
sudo ./hw_monitoring_program
```
