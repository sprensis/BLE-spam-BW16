# BW16 BLE Spam (RTL8720DN)

<p align="center">
  <img alt="project-logo" width="300" src="https://i.ibb.co/Q7gSf0FT/project-image.png">
</p>

[![Platform](https://img.shields.io/badge/Platform-BW16%20RTL8720dn-blue.svg)](https://www.amebaiot.com/en/amebad-bw16/)  
[![License](https://img.shields.io/badge/License-Educational-red.svg)](LICENSE)

---

A minimal Ameba Arduino (BW16 / RTL8720DN) project that advertises crafted BLE beacons for Apple (manufacturer frames), Microsoft, Google Fast Pair, and Samsung. It exposes a simple Wi‑Fi Access Point with an HTTP UI to start/stop spam and pick a payload type.

## Features
- BLE advertising using Ameba RTL8720DN APIs
- Wi‑Fi AP + web UI (English) on port `80`
- Payload types: `Apple`, `Microsoft`, `Google`, `Samsung`, `SourApple`
- Start/Stop control and status view
- Serial logging for diagnostics.

## Setup: Arduino IDE (BW16 / RTL8720DN)
1. Install Arduino IDE (2.x preferred)
2. Open `Arduino IDE → Preferences` and add this URL to `Additional Boards Manager URLs`:
   - `https://raw.githubusercontent.com/ambiot/ambd_arduino/master/Arduino_package/package_realtek_amebad_index.json`
3. Go to `Tools → Board → Boards Manager…` and search for `Realtek AmebaD`
4. Install the package for `AmebaD (RTL8720DN)`
5. Select the board: `Tools → Board → AmebaD [RTL8720DN] → BW16` (or `Generic AmebaD (RTL8720DN)` if BW16 is not listed)
6. Select the serial port of your BW16 under `Tools → Port`
7. Open this project in Arduino IDE and build/upload.

Notes:
- Required libraries (`WiFi`, `BLE`) are provided with the AmebaD board package.
- If compilation errors appear, ensure the AmebaD package is installed and selected; do not use ESP32 or other cores for BW16.

## Usage
- Power the board after flashing
- Connect to AP `BW16-Spam` (password `bw16spam`). You can change SSID/pass in `src/main/main.ino` (`AP_SSID`, `AP_PASS`). Note: ESP32 requires password length ≥ 8; Ameba uses secure AP without explicit channel and falls back to OPEN AP on channels 6/1/11 if WPA2 fails.
- Open `http://192.168.1.1/` in your browser (typical AP gateway). If it differs, check your device’s Wi‑Fi gateway IP
- Pick a type, click `Start` to begin advertising.
- Click `Stop` to halt advertising.

## Troubleshooting
- If you see errors like `random() expects 2 arguments`, ensure you compile with the Realtek AmebaD (RTL8720DN) core. This project uses the two‑argument `random(min, max)` form
- If `WiFi.apbegin(...)` type errors occur, verify you selected the AmebaD core and not ESP32. The project calls the AmebaD `apbegin(ssid, password, channel)` or `apbegin(ssid, channelStr)` APIs depending on core version
- If AP startup logs show `ioctl[RTKIOCSIWFREQ] error` followed by `ERROR: Operation failed!`, your core may ignore/require different channel argument formats in secure AP mode. The firmware now automatically falls back to OPEN AP and tries channels `6 → 1 → 11`. You should be able to connect to the open AP if WPA2 fails on your SDK.
- PlatformIO is not supported for BW16; use Arduino IDE.

## Notes
- TX power control is not exposed in Ameba Arduino BLE API
- MAC randomization at Arduino level may be limited; advertising uses platform MAC
- PlatformIO: BW16 (AmebaD) is not officially supported; `platformio.ini` here is a placeholder. Use Arduino IDE for builds.

## Disclaimer
This project is for educational and authorized testing purposes only. Misuse can be illegal and unethical. You are solely responsible for your actions.
