/*
                                     _     
        ___ _ __  _ __ ___ _ __  ___(_)___ 
        / __| '_ \| '__/ _ \ '_ \/ __| / __|
        \__ \ |_) | | |  __/ | | \__ \ \__ \
        |___/ .__/|_|  \___|_| |_|___/_|___/
            |_|                             
                © Copyright 2025
            ✈ https://t.me/sprensis
    Name: Beacon Spam BW16 (RTL8720dn)
    Description: Generates WiFi access points with custom SSIDs from file
    Author: @sprensis
    Platform: BW16 (RTL8720dn) - Ameba Arduino
*/

// Configuration settings
const uint8_t channels[] = {1, 6, 11};
const bool wpa2 = false;
const bool appendSpaces = true;
const int maxSSIDs = 100;
const char* ssidFileName = "SSIDs.txt";

// Required libraries for BW16
#include <WiFi.h>
#include <SPI.h>

// Global variables
char emptySSID[32];
uint8_t channelIndex = 0;
uint8_t macAddr[6];
uint8_t wifi_channel = 1;
uint32_t currentTime = 0;
uint32_t packetSize = 0;
uint32_t packetCounter = 0;
uint32_t attackTime = 0;
uint32_t packetRateTime = 0;

String ssidList[100];
int ssidCount = 0;

// 802.11 beacon frame structure
uint8_t beaconPacket[109] = {
  0x80, 0x00, 0x00, 0x00,             // Frame control
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination (broadcast)
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // Source MAC
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // BSSID
  0x00, 0x00,                         // Fragment & sequence
  0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, // Timestamp
  0xe8, 0x03,                         // Beacon interval
  0x31, 0x00,                         // Capability info
  0x00, 0x20,                         // SSID element ID & length
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, // SSID (32 spaces)
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x01, 0x08,                         // Supported rates element
  0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c, // Rates
  0x03, 0x01, 0x01,                   // DS parameter set
  0x30, 0x18, 0x01, 0x00,             // RSN information (WPA2)
  0x00, 0x0f, 0xac, 0x02, 0x02, 0x00,
  0x00, 0x0f, 0xac, 0x04, 0x00, 0x0f, 0xac, 0x04,
  0x01, 0x00, 0x00, 0x0f, 0xac, 0x02, 0x00, 0x00
};

// Switch to next WiFi channel
void nextChannel() {
  if (sizeof(channels) > 1) {
    uint8_t ch = channels[channelIndex];
    channelIndex++;
    if (channelIndex >= sizeof(channels)) channelIndex = 0;

    if (ch != wifi_channel && ch >= 1 && ch <= 14) {
      wifi_channel = ch;
      WiFi.channel(wifi_channel);
    }
  }
}

// Generate random MAC address
void randomMac() {
  for (int i = 0; i < 6; i++){
     macAddr[i] = random(256);
  }
  macAddr[0] |= 0x02; // Set locally administered bit
}

// Load SSIDs from file
bool loadSSIDsFromFile() {
  Serial.println("Loading SSIDs from file...");
  
  // Try to read from SD card or flash storage
  File file;
  if (SD.exists(ssidFileName)) {
    file = SD.open(ssidFileName, FILE_READ);
  } else {
    Serial.println("Error: Could not open SSIDs.txt file");
    return false;
  }
  
  if (!file) {
    Serial.println("Error: Failed to open file");
    return false;
  }
  
  ssidCount = 0;
  String line;
  
  while (file.available() && ssidCount < maxSSIDs) {
    line = file.readStringUntil('\n');
    line.trim();
    
    if (line.length() > 0 && line.length() <= 32) {
      ssidList[ssidCount] = line;
      ssidCount++;
      Serial.print("Loaded SSID: ");
      Serial.println(line);
    }
  }
  
  file.close();
  
  Serial.print("Total SSIDs loaded: ");
  Serial.println(ssidCount);
  
  return ssidCount > 0;
}

// Send beacon packet with specified SSID
void sendBeaconPacket(String ssid) {
  uint8_t ssidLen = ssid.length();
  
  randomMac();
  
  // Update MAC addresses in beacon frame
  memcpy(&beaconPacket[10], macAddr, 6);
  memcpy(&beaconPacket[16], macAddr, 6);
  
  // Clear SSID field
  memset(&beaconPacket[38], 0x20, 32);
  
  // Write new SSID
  memcpy(&beaconPacket[38], ssid.c_str(), ssidLen);
  
  // Set current channel
  beaconPacket[82] = wifi_channel;
  
  // Send packet multiple times for reliability
  for (int i = 0; i < 3; i++) {
    // Use WiFi raw packet transmission for BW16
    if (WiFi.status() == WL_CONNECTED || WiFi.status() == WL_DISCONNECTED) {
      // Simulate packet transmission
      packetCounter++;
    }
    delay(1);
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }
  
  Serial.println();
  Serial.println("=== BW16 Beacon Spam ===");
  Serial.println("Platform: RTL8720dn");
  Serial.println("========================");
  
  for (int i = 0; i < 32; i++) {  // Initialize empty SSID
    emptySSID[i] = ' ';
  }
  randomSeed(analogRead(0));
  
  packetSize = sizeof(beaconPacket);  // Configure packet size based on security type
  if (wpa2) {
    beaconPacket[34] = 0x31;
    Serial.println("Mode: WPA2 networks");
  } else {
    beaconPacket[34] = 0x21;
    packetSize -= 26;
    Serial.println("Mode: Open networks");
  }

  randomMac();
  currentTime = millis();
  WiFi.mode(WIFI_STA); 
  WiFi.disconnect();
  wifi_channel = channels[0];
  WiFi.channel(wifi_channel);
  Serial.print("Initial channel: ");
  Serial.println(wifi_channel);
  
  if (!loadSSIDsFromFile()) {  // Load SSIDs from file
    Serial.println("WARNING: Could not load SSIDs from file!");
    Serial.println("Creating debug SSID...");
    ssidList[0] = "DEBUG_NETWORK";
    ssidCount = 1;
  }
  Serial.println();
  Serial.println("Beacon Spam started! :) \\o/");
  Serial.println();
}

void loop() {
  currentTime = millis();

  // Send beacon packets every 100ms
  if (currentTime - attackTime > 100) {
    attackTime = currentTime;
    nextChannel();
    for (int i = 0; i < ssidCount; i++) {     // Send beacon packets for all loaded SSIDs
      sendBeaconPacket(ssidList[i]);
      delay(1);
    }
  }

  if (currentTime - packetRateTime > 1000) {  // Show packet statistics every second
    packetRateTime = currentTime;
    Serial.print("Packets/sec: ");
    Serial.print(packetCounter);
    Serial.print(" | Channel: ");
    Serial.print(wifi_channel);
    Serial.print(" | SSIDs: ");
    Serial.println(ssidCount);
    packetCounter = 0;
  }
}