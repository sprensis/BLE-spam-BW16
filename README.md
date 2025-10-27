# BW16 Beacon Spam - RTL8720dn

<p align="center">
  <img alt="project-logo" width="300" src="https://i.ibb.co/Q7gSf0FT/project-image.png">
</p>

[![Platform](https://img.shields.io/badge/Platform-BW16%20RTL8720dn-blue.svg)](https://www.amebaiot.com/en/amebad-bw16/)  
[![License](https://img.shields.io/badge/License-Educational-red.svg)](LICENSE)

---

## Disclaimer

**This tool is intended strictly for educational purposes and authorized penetration testing only.**

---

## Features

- **Real packet transmission** — uses the native RTL8720dn APIs for beacon frame injection.  
- **Multi-channel support** — automatic channel hopping across configured channels.  
- **Built-in SSIDs** — includes 10 predefined SSID names.  
- **MAC randomization** — generates a unique MAC address for each frame.  
- **Performance monitoring** — real-time statistics output via serial.

---

## Requirements

- **BW16 board** with the RTL8720dn chipset  
- **USB cable** for programming and power  
- **Arduino IDE** with the Ameba Arduino package installed

---

## Configuration

Edit configuration values in `Beacon_spam_main.ino` before compiling:

```cpp
const uint8_t channels[] = {1, 6, 11};  // Wi-Fi channels to use
const bool wpa2 = false;                // Enable/disable WPA2 flag in SSIDs (simulated)
const bool appendSpaces = true;         // Append spaces to SSID names (optional)
const int maxSSIDs = 100;               // Maximum number of SSIDs to generate/use
```

Additional options and built-in SSID lists can be found in the sketch files under `src/`.

---

## Usage

1. Power the BW16 board.
2. Open the Serial Monitor at **115200 baud**.
3. The device will automatically start transmitting beacon frames according to the configured settings.
4. Monitor packet counts, channel statistics, and other runtime information via serial output.

### Example serial output

```
========================================
    BW16 Beacon Spam v2.0
    Platform: RTL8720dn
    Author: @sprensis
========================================

[CONFIG] Mode: Open networks
[INFO] Monitor mode enabled
[INFO] Initial channel: 1
[INFO] Loading built-in SSIDs...
[INFO] Available SSID entries: /.../
[INFO] Successfully loaded /.../ SSIDs

[SUCCESS] Beacon spam initialized successfully!
[INFO] Broadcasting /.../ SSIDs across channels
[INFO] Press reset button to stop

[STATS] Rate: 45 pkt/s | Channel: 1 | SSIDs: /.../ | Total: 45
[STATS] Rate: 47 pkt/s | Channel: 6 | SSIDs: /.../ | Total: 92
```

---

## 🔍 Troubleshooting

* Ensure the Ameba Arduino package is installed and the BW16 (RTL8720dn) board is selected
* Verify the correct COM port is selected and the board is powered
* If uploads fail, try pressing the board’s boot/reset buttons as required by your hardware revision
* Serial output may require opening the Serial Monitor at **115200** baud
* For low packet rates or unexpected behavior, check other running tasks on the board and reduce SSID count or transmission interval

---

## Performance

* **Typical packet rate**: 40–50 packets per second
* **Channel switch time**: ~10 ms
* **RAM usage**: ~15 KB
* **Flash usage**: ~25 KB
* **Power consumption**: ~150 mA @ 3.3 V

---

## License

This project is licensed for educational use — see the [LICENSE](LICENSE) file for details.

---

## 📞 Support

* **Telegram**: [@cuudeen](https://t.me/сuudeen)
* **Issues**: Please report bugs or request features via GitHub Issues.

