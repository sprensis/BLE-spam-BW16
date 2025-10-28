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

#include <Arduino.h>
#include <WiFi.h>

#if defined(ESP32)
// Placeholder build stubs for BLE control on ESP32 env
enum EBLEPayloadType { AppleJuice, Google, Microsoft, Samsung, SourApple, AllBurst, AndroidBurst };
 static bool gFakeActive = false;
 inline void startSpamStub(EBLEPayloadType){ gFakeActive = true; }
 inline void stopSpamStub(){ gFakeActive = false; }
 inline bool isSpamActiveStub(){ return gFakeActive; }
 inline void tickSpamStub(){ /* no-op */ }
 #define startSpam(t) startSpamStub(t)
 #define stopSpam() stopSpamStub()
 #define isSpamActive() isSpamActiveStub()
 #define tickSpam() tickSpamStub()
#else
#include "ble_spam.h"
#endif

// Simple Wi‑Fi AP + HTTP server UI to control BLE spam
// Tested with Ameba RTL8720DN Arduino core APIs

// AP configuration
static const char* AP_SSID = "BW16-Spam";
static const char* AP_PASS = "bw16spam";   // min 8 chars; set empty string for open AP
static const uint8_t AP_CHANNEL = 6;        // typical 1/6/11

WiFiServer server(80);

// LED / status flags
static bool gApUp = false;
#ifdef LED_BUILTIN
static bool gLedState = false;
#endif

// Current selected spam type; default to AppleJuice
static EBLEPayloadType currentType = AppleJuice;

String htmlHeader() {
  return String(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=utf-8\r\n"
    "Connection: close\r\n\r\n");
}

// URL decode helper (percent-encoding and '+')
String urlDecode(const String& s) {
  String out; out.reserve(s.length());
  for (size_t i = 0; i < s.length(); ++i) {
    char c = s[i];
    if (c == '+') { out += ' '; }
    else if (c == '%' && i + 2 < s.length()) {
      char h1 = s[i+1], h2 = s[i+2];
      auto hex = [](char h){ if (h >= '0' && h <= '9') return h - '0'; if (h >= 'A' && h <= 'F') return h - 'A' + 10; if (h >= 'a' && h <= 'f') return h - 'a' + 10; return 0; };
      char val = (char)((hex(h1) << 4) | hex(h2));
      out += val; i += 2;
    } else { out += c; }
  }
  return out;
}

#ifdef LED_BUILTIN
void bootBlink() {
  pinMode(LED_BUILTIN, OUTPUT);
  for (int i = 0; i < 3; ++i) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(120);
    digitalWrite(LED_BUILTIN, LOW);
    delay(120);
  }
}

void updateStatusLed() {
  static unsigned long last = 0;
  unsigned long now = millis();
  if (isSpamActive()) {
    if (now - last >= 500) { // slow blink when advertising
      last = now;
      gLedState = !gLedState;
      digitalWrite(LED_BUILTIN, gLedState ? HIGH : LOW);
    }
  } else if (gApUp) {
    digitalWrite(LED_BUILTIN, HIGH); // steady on when AP up
  } else {
    if (now - last >= 200) { // fast blink on AP error
      last = now;
      gLedState = !gLedState;
      digitalWrite(LED_BUILTIN, gLedState ? HIGH : LOW);
    }
  }
}
#endif

void sendIndex(WiFiClient &client) {
  bool active = isSpamActive();
  const char* status = active ? "Active" : "Stopped";
  const char* typeName = "Apple";
  switch (currentType) {
    case AppleJuice: typeName = "Apple"; break;
    case Google: typeName = "Android/Google"; break;
    case Microsoft: typeName = "Windows/Microsoft"; break;
    case Samsung: typeName = "Samsung"; break;
    case SourApple: typeName = "SourApple"; break;
    case AndroidBurst: typeName = "Android Burst"; break;
    default: typeName = "Unknown"; break;
  }

  client.print(htmlHeader());
  client.print("<!doctype html><html lang=\"en\"><head><meta charset=\"utf-8\">");
  client.print("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.print("<title>BW16 BLE Spam</title>");
  client.print("<style>:root{color-scheme:dark}*{box-sizing:border-box}body{margin:0;padding:16px;background:#0b0f15;color:#e5e7eb;font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial}.container{max-width:720px;margin:0 auto}h1{font-size:20px;margin:0 0 12px}.card{background:#111827;border:1px solid #1f2937;border-radius:12px;padding:14px;margin-top:12px;box-shadow:0 1px 3px rgba(0,0,0,.3)}.status{font-weight:600}.controls{display:flex;flex-wrap:wrap;gap:8px;margin-top:10px}button{appearance:none;border:1px solid #374151;background:#1f2937;color:#e5e7eb;border-radius:10px;padding:10px 14px;cursor:pointer}button:hover{background:#374151}select,input{background:#0f172a;color:#e5e7eb;border:1px solid #374151;border-radius:10px;padding:8px;width:100%}@media(min-width:480px){select{width:auto}}.grid{display:grid;grid-template-columns:1fr;gap:12px}@media(min-width:720px){.grid{grid-template-columns:1fr 1fr}}.footer{color:#9ca3af;font-size:12px;margin-top:8px}</style>");
  client.print("</head><body><div class=\"container\"><h1>BW16 RTL8720DN — BLE Spam</h1>");
  client.print("<div class=\"grid\">");

  // Status card
  client.print("<div class=\"card\"><div>Status: <span class=\"status\">");
  client.print(status);
  client.print("</span></div><div>Type: <b>");
  client.print(typeName);
  client.print("</b></div><div class=\"controls\"><a href=\"/start\"><button>Start</button></a> <a href=\"/stop\"><button>Stop</button></a> <a href=\"/start_all\"><button>Start All/Burst</button></a> <a href=\"/start_android\"><button>Start Android Burst</button></a></div></div>");

  // Config card
  client.print("<div class=\"card\"><div>Configure spam type</div><form action=\"/set\" method=\"get\" style=\"margin-top:8px\">");
  client.print("<label>Type: </label><select name=\"type\">");
  client.print("<option value=\"apple\""); if (currentType == AppleJuice) client.print(" selected"); client.print(">Apple</option>");
  client.print("<option value=\"google\""); if (currentType == Google) client.print(" selected"); client.print(">Android / Google</option>");
  client.print("<option value=\"android\""); if (currentType == AndroidBurst) client.print(" selected"); client.print(">Android Burst</option>");
  client.print("<option value=\"microsoft\""); if (currentType == Microsoft) client.print(" selected"); client.print(">Windows</option>");
  client.print("<option value=\"samsung\""); if (currentType == Samsung) client.print(" selected"); client.print(">Samsung</option>");
  client.print("<option value=\"sourapple\""); if (currentType == SourApple) client.print(" selected"); client.print(">SourApple</option>");
  client.print("<option value=\"all\""); if (currentType == AllBurst) client.print(" selected"); client.print(">All / Burst</option>");
  client.print("</select> <button type=\"submit\">Save</button></form>");
  client.print("<p class=\"footer\">Advertising mode: non-connectable. TX power is platform-defined.</p>");
  client.print("</div>");

  client.print("</div></div></body></html>");
}

EBLEPayloadType parseTypeParam(const String& v) {
  if (v == "apple") return AppleJuice;
  if (v == "google") return Google;
  if (v == "android") return AndroidBurst;
  if (v == "microsoft") return Microsoft;
  if (v == "samsung") return Samsung;
  if (v == "sourapple") return SourApple;
  if (v == "all") return AllBurst;
  return AppleJuice;
}

void sendRedirect(WiFiClient &client, const String &location) {
  client.print("HTTP/1.1 302 Found\r\n");
  client.print("Content-Type: text/html; charset=utf-8\r\n");
  client.print("Location: "); client.print(location); client.print("\r\n");
  client.print("Connection: close\r\n\r\n");
  client.print("<!doctype html><html><head><meta charset=\"utf-8\"><title>Redirect</title></head><body>Redirecting to <a href=\"");
  client.print(location);
  client.print("\">");
  client.print(location);
  client.print("</a>...</body></html>");
}

void handleHttpClient(WiFiClient &client) {
  // Read request line
  String reqLine = client.readStringUntil('\n');
  reqLine.trim(); // remove trailing CR
  Serial.print("Request: "); Serial.println(reqLine);
  // Skip rest of headers quickly
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) break; // blank line ends headers
  }

  // Parse method and path
  int sp1 = reqLine.indexOf(' ');
  int sp2 = reqLine.indexOf(' ', sp1 + 1);
  if (sp1 < 0 || sp2 < 0) {
    sendIndex(client);
    return;
  }
  String method = reqLine.substring(0, sp1);
  String fullPath = reqLine.substring(sp1 + 1, sp2);

  // Split route and query
  String route = fullPath;
  String query;
  int q = fullPath.indexOf('?');
  if (q >= 0) {
    route = fullPath.substring(0, q);
    query = fullPath.substring(q + 1);
  }

  Serial.print("Route: "); Serial.println(route);

  if (route == "/start") {
    startSpam(currentType);
    sendRedirect(client, "/");
    return;
  }
  if (route == "/start_all") {
    currentType = AllBurst;
    startSpam(currentType);
    sendRedirect(client, "/");
    return;
  }
  if (route == "/start_android") {
    currentType = AndroidBurst;
    startSpam(currentType);
    sendRedirect(client, "/");
    return;
  }
  if (route == "/stop") {
    stopSpam();
    sendRedirect(client, "/");
    return;
  }
  if (route == "/set") {
    // parse simple query: type=...
    String t;
    int p = query.indexOf("type=");
    if (p >= 0) {
      t = query.substring(p + 5);
      int amp = t.indexOf('&');
      if (amp >= 0) t = t.substring(0, amp);
      t = urlDecode(t);
    }
    if (t.length()) {
      currentType = parseTypeParam(t);
      Serial.print("Set type: "); Serial.println(t);
    }
    sendRedirect(client, "/");
    return;
  }

  // default index
  sendIndex(client);
}

void setupAP() {
  // Start AP. If password empty => open AP variant
#if defined(ESP32)
  if (AP_PASS && strlen(AP_PASS) >= 8) {
    WiFi.softAP(AP_SSID, AP_PASS, AP_CHANNEL);
  } else {
    Serial.println("AP: пароль < 8 символов, включен открытый AP");
    WiFi.softAP(AP_SSID, "", AP_CHANNEL);
  }
  gApUp = true; // assume success on ESP32 demo
#else
  // Ameba RTL8720DN WiFi: use documented signatures
  char* ssid = const_cast<char*>(AP_SSID);
  size_t passLen = (AP_PASS ? strlen(AP_PASS) : 0);
  int rc = 0;

  if (passLen >= 8) {
    Serial.println("AP: secure (no channel)");
    rc = WiFi.apbegin(ssid, const_cast<char*>(AP_PASS));
    if (rc != 1) {
      Serial.println("AP: secure failed, fallback open ch6");
      char ch6[] = "6"; char ch1[] = "1"; char ch11[] = "11";
      rc = WiFi.apbegin(ssid, ch6);
      if (rc != 1) { Serial.println("AP: retry open ch1"); rc = WiFi.apbegin(ssid, ch1); }
      if (rc != 1) { Serial.println("AP: retry open ch11"); rc = WiFi.apbegin(ssid, ch11); }
    }
  } else {
    Serial.println("AP: open, ch6");
    char ch6[] = "6"; char ch1[] = "1"; char ch11[] = "11";
    rc = WiFi.apbegin(ssid, ch6);
    if (rc != 1) { Serial.println("AP: retry open ch1"); rc = WiFi.apbegin(ssid, ch1); }
    if (rc != 1) { Serial.println("AP: retry open ch11"); rc = WiFi.apbegin(ssid, ch11); }
  }

  gApUp = (rc == 1);
  Serial.println(gApUp ? "AP: started" : "AP: failed");
#endif
  server.begin();
}

void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println("Booting BW16 BLE Spam...");
  randomSeed(((uint32_t)micros()) ^ 0xA5A5A5A5);
#ifdef LED_BUILTIN
  bootBlink();
#endif
  // BLE idle on boot
  stopSpam();

  setupAP();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    client.setTimeout(2000);
    handleHttpClient(client);
    delay(2);
    client.stop();
  }

#ifdef LED_BUILTIN
  updateStatusLed();
#endif

  // Drive burst rotation if enabled
  tickSpam();

  // Small idle delay
  delay(10);
}