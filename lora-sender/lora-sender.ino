#include <HardwareSerial.h>

// //Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>

// //Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// //define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

// //433E6 for Asia
// //866E6 for Europe
// //915E6 for North America
#define BAND 433E6

// //OLED pins
#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels


// //packet counter
int readingID = 0;

int counter = 0;
String LoRaMessage = "";



Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

// //Initialize OLED display
void startOLED(){
  //reset OLED display via software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("LORA SENDER");
}

// //Initialize LoRa module
void startLoRA(){
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
    readingID++;
    Serial.println("Starting LoRa failed!"); 
  }
  Serial.println("LoRa Initialization OK!");
  display.setCursor(0,10);
  display.clearDisplay();
  display.print("LoRa Initializing OK!");
  display.display();
  delay(2000);
}

void sendReadings(String location) {
  LoRaMessage = location;
  //Send LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.print(LoRaMessage);
  LoRa.endPacket();
  
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.print("LoRa packet sent!");
  display.setCursor(0,20);
  display.print("Location:");
  display.setCursor(72,20);
  display.print(location);
  display.print("Reading ID:");
  display.setCursor(66,50);
  display.print(readingID);
  display.display();
  Serial.print("Sending packet: ");
  Serial.println(readingID);
  readingID++;
}



// void setup() {
//   // Initialize Serial on UART2 (pins 16 and 17 on TTGO LoRa32 V1.0, but check your board's pinout)
//   // RX = 16, TX = 17 for TTGO LoRa32 V1.0. Adjust for your board version if needed.
//   Serial2.begin(115200, SERIAL_8N1, 16, 17);
//   Serial.begin(115200); // Start the main serial for debugging
//   startOLED();
//   // startBME();
//   startLoRA();
// }

// void loop() {
//   if (Serial2.available()) {
//     // Read the incoming byte:
//     String location = Serial2.readString();
//     // Display the incoming message on the main serial
//     Serial.print("I received: ");
//     Serial.println(location);
//     sendReadings(location);
//     delay(5000);
//   }
// }
String incomingMessage = "";
void setup() {
  // Initialize Serial on UART2 (pins 16 and 17 on TTGO LoRa32 V1.0, but check your board's pinout)
  // RX = 13, TX = 17 for TTGO LoRa32 V1.0. Adjust for your board version if needed.
  Serial2.begin(115200, SERIAL_8N1, 13, 17);
  Serial.begin(115200); // Start the main serial for debugging
  startOLED();
  startLoRA();
}

void loop() {
  if (Serial2.available()) {
    // Read the incoming byte:

    incomingMessage = Serial2.readString();
    //  incomingMessage = "$GPGGA,114135.00,2002.5009,N,09952.9638,E,1,05,2.0,381.5,M,,M,,*76";
    
    // Display the incoming message on the main serial
    Serial.print("I received: ");
    Serial.println(incomingMessage);
    sendReadings(incomingMessage);
    // sendReadings(incomingMessage);
    delay(1000);
  }
    // incomingMessage = "$GPGGA,114135.00,2002.5009,N,09952.9638,E,1,05,2.0,381.5,M,,M,,*76";
    // Serial.println(incomingMessage);
    // delay(1000);
}