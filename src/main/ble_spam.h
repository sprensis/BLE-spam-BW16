/*
                                     _     
        ___ _ __  _ __ ___ _ __  ___(_)___ 
        / __| '_ \| '__/ _ \ '_ \/ __| / __|
        \__ \ |_) | | |  __/ | | \__ \ \__ \
        |___/ .__/|_|  \___|_| |_|___/_|___/
            |_|                             
                © Copyright 2025
            ✈ https://github.com/sprensis
    Name: BLE-spam-BW16 (RTL8720dn)
    Description: Spams multiple BLE access points with custom SSIDs on all 2.4 GHz channels (1–11).
    Author: @sprensis
    Platform: BW16 (RTL8720dn) - Ameba Arduino
    License: MIT
*/

#pragma once
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEAdvert.h>
#include <BLEAdvertData.h>

// Enum for BLE payload types
enum EBLEPayloadType { Microsoft, SourApple, AppleJuice, Samsung, Google, AllBurst, AndroidBurst };

// Structure for devices (Android and Samsung watches)
struct Device {
    const char* name;
    uint32_t code;
};

// Structure for Apple device mapping
struct AppleDeviceMapping {
    uint8_t* packet;
    uint8_t size;
    const char* name;
};

// IOS-Devices (declarations)
extern uint8_t Airpods[31];
extern uint8_t AirpodsPro[31];
extern uint8_t AirpodsMax[31];
extern uint8_t AirpodsGen2[31];
extern uint8_t AirpodsGen3[31];
extern uint8_t AirpodsProGen2[31];
extern uint8_t PowerBeats[31];
extern uint8_t PowerBeatsPro[31];
extern uint8_t BeatsSoloPro[31];
extern uint8_t BeatsStudioBuds[31];
extern uint8_t BeatsFlex[31];
extern uint8_t BeatsX[31];
extern uint8_t BeatsSolo3[31];
extern uint8_t BeatsStudio3[31];
extern uint8_t BeatsStudioPro[31];
extern uint8_t BeatsFitPro[31];
extern uint8_t BeatsStudioBudsPlus[31];
extern uint8_t AppleTVSetup[23];
extern uint8_t AppleTVPair[23];
extern uint8_t AppleTVNewUser[23];
extern uint8_t AppleTVAppleIDSetup[23];
extern uint8_t AppleTVWirelessAudioSync[23];
extern uint8_t AppleTVHomekitSetup[23];
extern uint8_t AppleTVKeyboard[23];
extern uint8_t AppleTVConnectingToNetwork[23];
extern uint8_t HomepodSetup[23];
extern uint8_t SetupNewPhone[23];
extern uint8_t TransferNumber[23];
extern uint8_t TVColorBalance[23];
extern uint8_t AppleVisionPro[23];

// Android-Devices
extern const Device devices[];
extern const int devicesCount;

// Apple Devices Mapping
extern const AppleDeviceMapping appleDevices[];
extern const int appleDevicesCount;

// Function prototypes
void aj_adv(int ble_choice);
void ibeacon(const char* DeviceName = "ESP-HACK", const char* BEACON_UUID = "8ec76ea3-6668-48da-9866-75be8bc86f4d", int ManufacturerId = 0x4C00);
void executeSpam(EBLEPayloadType type);
void generateRandomMac(uint8_t *mac);
BLEAdvertData GetUniversalAdvertisementData(EBLEPayloadType Type);
// New control APIs for BW16 Ameba BLE
void startSpam(EBLEPayloadType type);
void stopSpam();
bool isSpamActive();
void tickSpam();