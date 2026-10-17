# Ultrasonic Radar (ESP32)

A 2D radar that scans 0–180° with an HC-SR04 ultrasonic sensor mounted on a servo, and shows the result live on a military-style display built in **Processing**.
Version 1.1 detects obstacles within **15 cm**.

<!-- Add your photos and a short GIF of the display here:
![Radar hardware](docs/radar.jpg)
![ESP32 board](docs/esp32.jpg)
-->

## Hardware

| Part | Role |
|---|---|
| ESP32-WROOM-32 | Controls servo angle, triggers the sensor, measures echo time, filters readings and sends data over serial |
| HC-SR04 | Ultrasonic distance sensor |
| SG90 micro servo (9 g) | Rotates the sensor 0–180° |
| LM2596 buck converter | Independent, stable 5 V supply (calibrated to 5.05 V) fed from a 9 V adapter |

### Pin map
| Signal | ESP32 pin |
|---|---|
| Servo signal | GPIO 13 |
| HC-SR04 TRIG | GPIO 5 |
| HC-SR04 ECHO | GPIO 18 |

> ⚠️ HC-SR04 Echo outputs 5 V while ESP32 GPIOs are 3.3 V tolerant. Use a voltage divider on the Echo line.

### Power design (problem → fix)
Powering the ESP32 and servo from the computer's USB caused sudden voltage drops and repeated resets.
**Fix:** the servo and sensor run from a separate LM2596 supply, and the ESP32 GND is tied to the LM2596 negative output (**common ground**) so signals share a reference.

## Software

### Firmware (`firmware/radar_esp32`, Arduino IDE + `ESP32Servo`)
- Servo sweeps 0→180→0 in **1° steps** with a **35 ms** settle delay
- **3 readings per angle**: a majority must see an object (single spikes are rejected), readings far from the median are dropped, the rest are averaged
- Short echo timeout so empty space does not slow the sweep
- No echo / out of range is reported as the max range (15 cm)
- Sends `angle,distance` lines over serial at **115200 baud**
- All tunable values (pins, range, delays, samples, tolerance) are named constants at the top of the sketch
- The filter lives in `radar_filter.h` with no Arduino dependency, so it can be tested on a PC

### Display (`processing/radar_display`, Processing 4)
- Opens the serial port selected by `PORT_INDEX` (the available ports are printed to the console)
- Green scanning line with fading trails
- 181-cell memory array (one per degree): detected points (red) stay and fade gradually after the scan line passes
- Calibrated rings for 5, 10 and 15 cm with labels

## Setup
1. In the Arduino IDE install the **ESP32** board package and the **ESP32Servo** library, then upload `firmware/radar_esp32/radar_esp32.ino`.
2. Close the Arduino Serial Monitor (only one program can use the port).
3. Open `processing/radar_display/radar_display.pde` in Processing and press Run.
4. If nothing appears, check the console for the port list and change `PORT_INDEX` in the sketch.

## Test the filter on a PC
```bash
cd tests
g++ -std=c++11 -I../firmware/radar_esp32 test_filter.cpp -o test_filter && ./test_filter
```
This only tests the filtering logic. The sensor, servo and display need the real hardware.

## Repository layout
```
ultrasonic-radar/
├── firmware/radar_esp32/
│   ├── radar_esp32.ino
│   └── radar_filter.h
├── processing/radar_display/radar_display.pde
├── tests/test_filter.cpp
├── docs/            (report PDF, wiring diagram, photos)
└── README.md
```
In Arduino and Processing the folder name must match the main file name.

## Roadmap (v2.0)
- [ ] Extend range to ~1 m (adjust the Echo voltage divider and the timeout)
- [ ] Wireless data via ESP32 Wi-Fi instead of USB serial
- [ ] Faster sweep while keeping accuracy
- [ ] Separate "no object" from "out of range" values in the serial protocol
- [ ] Wiring diagram (Fritzing) and a demo GIF

## Author
Moghtaderi
