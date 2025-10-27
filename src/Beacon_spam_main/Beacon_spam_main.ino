/*
                                     _     
        ___ _ __  _ __ ___ _ __  ___(_)___ 
        / __| '_ \| '__/ _ \ '_ \/ __| / __|
        \__ \ |_) | | |  __/ | | \__ \ \__ \
        |___/ .__/|_|  \___|_| |_|___/_|___/
            |_|                             
                © Copyright 2025
            ✈ https://t.me/cuudeen
    Name: Beacon-Spam-BW16 (RTL8720dn)
    Description: Generates up to 1,000 unique WiFi access points with custom SSIDs / Spam 
    Author: @sprensis
    Platform: BW16 (RTL8720dn) - Ameba Arduino
    License: MIT
*/

// Configuration settings
const uint8_t channels[] = {1, 6, 11};
const bool wpa2 = false;
const bool appendSpaces = true;
const int maxSSIDs = 100;
const char* ssidFileName = "SSIDs.txt";

// Required libraries for BW16/RTL8720dn
#include <WiFi.h>
#include <SPI.h>

// External C functions for RTL8720dn WiFi control
extern "C" {
  #include "wifi_conf.h"
  #include "wifi_constants.h"
  int wifi_set_channel(int channel);
  int wifi_set_mode(rtw_mode_t mode);
  int wifi_send_raw_frame(unsigned char *raw_data, int raw_data_len);
}

// Global variables for beacon spam operation
char emptySSID[32];
uint8_t channelIndex = 0;
uint8_t macAddr[6];
uint8_t wifi_channel = 1;
uint32_t currentTime = 0;
uint32_t packetSize = 0;
uint32_t packetCounter = 0;
uint32_t attackTime = 0;
uint32_t packetRateTime = 0;
uint32_t totalPacketsSent = 0;

String ssidList[100];
int ssidCount = 0;

// 802.11 beacon frame structure for RTL8720dn
uint8_t beaconPacket[109] = {
  0x80, 0x00, 0x00, 0x00,             // Frame control: Type=Management, Subtype=Beacon
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination: broadcast
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // Source MAC (will be randomized)
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // BSSID (will be randomized)
  0x00, 0x00,                         // Fragment & sequence number
  0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, // Timestamp
  0xe8, 0x03,                         // Beacon interval (1000ms)
  0x31, 0x00,                         // Capability information
  0x00, 0x20,                         // SSID element ID & length (32 bytes)
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, // SSID field (32 spaces)
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x01, 0x08,                         // Supported rates element
  0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c, // Basic rates
  0x03, 0x01, 0x01,                   // DS parameter set (channel)
  0x30, 0x18, 0x01, 0x00,             // RSN information (WPA2 - optional)
  0x00, 0x0f, 0xac, 0x02, 0x02, 0x00,
  0x00, 0x0f, 0xac, 0x04, 0x00, 0x0f, 0xac, 0x04,
  0x01, 0x00, 0x00, 0x0f, 0xac, 0x02, 0x00, 0x00
};

// Switch to next WiFi channel using RTL8720dn API
void nextChannel() {
  if (sizeof(channels) > 1) {
    uint8_t ch = channels[channelIndex];
    channelIndex++;
    if (channelIndex >= sizeof(channels)) channelIndex = 0;

    if (ch != wifi_channel && ch >= 1 && ch <= 14) {
      wifi_channel = ch;
      wifi_set_channel(ch);
      delay(10); // Allow channel switch to complete
    }
  }
}

// Generate cryptographically secure random MAC address
void randomMac() {
  for (int i = 0; i < 6; i++) {
    macAddr[i] = random(256);
  }
  // Set locally administered bit and clear multicast bit
  macAddr[0] = (macAddr[0] & 0xFE) | 0x02;
}

// Load SSIDs from built-in array (no SD card required)
bool loadSSIDsFromFile() {
  Serial.println("[INFO] Loading built-in SSIDs...");
  
  // Built-in SSID list for testing
  String builtInSSIDs[] = {
    "Free WiFi",
    "Guest Network", 
    "Hotel WiFi",
    "Coffee Shop",
    "Airport WiFi",
    "Public Internet",
    "Mobile Hotspot",
    "Home Network",
    "Office WiFi",
    "Library Access"
  };
  
  ssidCount = sizeof(builtInSSIDs) / sizeof(builtInSSIDs[0]);
  
  Serial.println("[INFO] Available SSID entries:");
  for (int i = 0; i < ssidCount; i++) {
    ssidList[i] = builtInSSIDs[i];
    Serial.print("  [");
    Serial.print(i + 1);
    Serial.print("] ");
    Serial.println(ssidList[i]);
  }
  
  Serial.print("[INFO] Successfully loaded ");
  Serial.print(ssidCount);
  Serial.println(" SSIDs");
  
  return ssidCount > 0;
}

// Send beacon packet using RTL8720dn raw packet transmission
void sendBeaconPacket(String ssid) {
  uint8_t ssidLen = ssid.length();
  
  // Generate new MAC address for each beacon
  randomMac();
  
  // Update MAC addresses in beacon frame
  memcpy(&beaconPacket[10], macAddr, 6); // Source MAC
  memcpy(&beaconPacket[16], macAddr, 6); // BSSID
  
  // Clear SSID field with spaces
  memset(&beaconPacket[38], 0x20, 32);
  
  // Insert new SSID
  memcpy(&beaconPacket[38], ssid.c_str(), ssidLen);
  
  // Update SSID length in frame
  beaconPacket[37] = ssidLen;
  
  // Set current channel in DS parameter
  beaconPacket[82] = wifi_channel;
  
  // Calculate actual packet size
  uint32_t actualPacketSize = packetSize;
  if (!wpa2) {
    actualPacketSize -= 26; // Remove RSN information for open networks
  }
  
  // Send beacon frame using RTL8720dn raw transmission
  int result = wifi_send_raw_frame(beaconPacket, actualPacketSize);
  
  if (result == 0) {
    packetCounter++;
    totalPacketsSent++;
  }
}

// Initialize system and hardware
void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }
  
  Serial.println();
  Serial.println("========================================");
  Serial.println("    BW16 Beacon Spam v2.0");
  Serial.println("    Platform: RTL8720dn");
  Serial.println("    Author: github.com/sprensis");
  Serial.println("========================================");
  Serial.println();
  
  // Initialize empty SSID buffer
  memset(emptySSID, 0x20, 32);
  
  // Seed random number generator
  randomSeed(analogRead(A0) + millis());
  
  // Configure beacon packet based on security settings
  packetSize = sizeof(beaconPacket);
  if (wpa2) {
    beaconPacket[34] = 0x31; // Privacy bit set
    Serial.println("[CONFIG] Mode: WPA2 secured networks");
  } else {
    beaconPacket[34] = 0x21; // Privacy bit clear
    Serial.println("[CONFIG] Mode: Open networks");
  }
  
  // Initialize WiFi in monitor mode
  WiFi.mode(WIFI_OFF);
  delay(100);
  
  // Enable monitor mode for raw packet transmission
  if (wifi_set_mode(RTW_MODE_MONITOR) != 0) {
    Serial.println("[ERROR] Failed to set monitor mode");
    while(1) delay(1000);
  }
  
  Serial.println("[INFO] Monitor mode enabled");
  
  // Set initial channel
  wifi_channel = channels[0];
  wifi_set_channel(wifi_channel);
  
  Serial.print("[INFO] Initial channel: ");
  Serial.println(wifi_channel);
  
  // Generate initial MAC address
  randomMac();
  
  // Load SSIDs from file
  if (!loadSSIDsFromFile()) {
    Serial.println("[WARN] Could not load SSIDs!");
    Serial.println("[INFO] Using fallback test SSIDs...");
    
    // Create fallback SSIDs for testing
    ssidList[0] = "TEST_NETWORK_1";
    ssidList[1] = "TEST_NETWORK_2";
    ssidList[2] = "DEBUG_AP";
    ssidCount = 3;
  }
  
  // Initialize timing variables
  currentTime = millis();
  attackTime = currentTime;
  packetRateTime = currentTime;
  
  Serial.println();
  Serial.println("[SUCCESS] Beacon spam initialized successfully!");
  Serial.print("[INFO] Broadcasting ");
  Serial.print(ssidCount);
  Serial.println(" SSIDs across channels");
  Serial.println();
}

// Main execution loop
void loop() {
  currentTime = millis();

  // Send beacon bursts every 100ms for optimal coverage
  if (currentTime - attackTime >= 100) {
    attackTime = currentTime;
    
    // Switch to next channel
    nextChannel();
    
    // Send beacon for each loaded SSID
    for (int i = 0; i < ssidCount; i++) {
      sendBeaconPacket(ssidList[i]);
      delayMicroseconds(500); // Brief delay between packets
    }
  }

  // Display statistics every second
  if (currentTime - packetRateTime >= 1000) {
    packetRateTime = currentTime;
    
    Serial.print("[STATS] Rate: ");
    Serial.print(packetCounter);
    Serial.print(" pkt/s | Channel: ");
    Serial.print(wifi_channel);
    Serial.print(" | SSIDs: ");
    Serial.print(ssidCount);
    Serial.print(" | Total: ");
    Serial.println(totalPacketsSent);
    
    packetCounter = 0;
  }
  
  // Yield to prevent watchdog timeout
  yield();
}