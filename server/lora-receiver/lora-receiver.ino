#include <WiFi.h>
#include "ESPAsyncWebServer.h"

#include <string>
#include <SPIFFS.h>

//Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>

//Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Libraries to get time from NTP Server
#include <NTPClient.h>
#include <WiFiUdp.h>

//define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define BAND 433E6

//OLED pins
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Replace with your network credentials
const char *ssid = "Galaxy A305393";
const char *password = "03258769p";

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedDate;
String day;
String hour;
String timestamp;


// Initialize variables to get and save LoRa data
int rssi;
String loRaMessage;
String latitude;
String longitude;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

//Initialize OLED display
void startOLED() {
  //reset OLED display via software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) {  // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("LORA SENDER");
}

//Initialize LoRa module
void startLoRA() {
  int counter;
  //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);

  while (!LoRa.begin(BAND) && counter < 10) {
    Serial.print(".");
    counter++;
    delay(500);
  }
  if (counter == 10) {
    // Increment readingID on every new reading
    Serial.println("Starting LoRa failed!");
  }
  Serial.println("LoRa Initialization OK!");
  display.setCursor(0, 10);
  display.clearDisplay();
  display.print("LoRa Initializing OK!");
  display.display();
  delay(2000);
}

void connectWiFi() {
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  display.setCursor(0, 20);
  display.print("Access web server at: ");
  display.setCursor(0, 30);
  display.print(WiFi.localIP());
  display.display();
}

// Read LoRa packet and get the sensor readings
void getLoRaData() {
  Serial.print("LoRa packet received: ");

  // Read LoRa packet
  while (LoRa.available()) {
    loRaMessage = LoRa.readString();
  }
  // Print LoRa packet (optional)
  Serial.print(loRaMessage);


  // Get RSSI
  rssi = LoRa.packetRssi();
  Serial.print(" with RSSI ");
  Serial.println(rssi);
}

// Function to get date and time from NTPClient
void getTimeStamp() {
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  Serial.println(formattedDate);

  // Extract date
  int splitT = formattedDate.indexOf("T");
  day = formattedDate.substring(0, splitT);
  Serial.println(day);
  // Extract time
  hour = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
  Serial.println(hour);
  timestamp = day + " " + hour;
}

// // Function to split LoRa data and extract latitude and longitude
void extractLocation() {
  // Ensure LoRaData is populated before processing
  if (loRaMessage.isEmpty()) {
    Serial.println("LoRa data not received yet");
    return;
  }

  // $GPGGA,114135.00,2002.5009,N,09952.9638,E,1,05,2.0,381.5,M,,M,,*76
  latitude = loRaMessage.substring(17,26);
  longitude = loRaMessage.substring(29,39);
  // display.clearDisplay();
  display.setCursor(0, 40);
  display.print("Latitude: ");
  display.print(latitude);
  display.setCursor(0, 50);
  display.print("Longitude: ");
  display.print(longitude);
  display.display();
}

const char *index_html = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8' /><meta name='viewport' content='width=device-width, initial-scale=1.0' /><title>Location</title></head><body><header><h2>ESP32 (LoRa + Server)</h2><p><strong>Last received packet:<br /><span id='timestamp'>%TIMESTAMP%</span></strong></p><p>LoRa RSSI: <span id='rssi'>%RSSI%</span></p></header><main><p>Locations: <span id='locations'>%LOCATIONS%</span></p></main><script>setInterval(updateValues, 10000, 'rssi');setInterval(updateValues, 10000, 'timestamp');setInterval(updateValues, 10000, 'locations');function updateValues(value) {var xhttp = new XMLHttpRequest();xhttp.onreadystatechange = function () {if (this.readyState == 4 && this.status == 200) {document.getElementById(value).innerHTML = this.responseText;}};xhttp.open('GET', '/' + value, true);xhttp.send();}</script></body></html>";
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  startOLED();
  startLoRA();
  connectWiFi();
  SPIFFS.begin();
  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
  });

  server.on("/locations", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(200);
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
    String location = latitude + " " +longitude ;

    // Send the combined string as the response
    request->send(200, "text/plain", location);
  });
  // server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   request->send_P(200, "text/plain", humidity.c_str());
  // });
  // server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   request->send_P(200, "text/plain", pressure.c_str());
  // });


  server.on("/timestamp", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", timestamp.c_str());
  });
  server.on("/rssi", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", String(rssi).c_str());
  });
  // server.on("/winter", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   request->send(SPIFFS, "/winter.jpg", "image/jpg");
  // });
  // Start server
  server.begin();

  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(0);
}

void loop() {
  // Check if there are LoRa packets available
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    getLoRaData();
    extractLocation();
    getTimeStamp();
  }
}