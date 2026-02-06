#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// inicializace displeje s použitím I2C (SDA=4, SCL=15)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


#ifndef STASSID  // toto je pouze pro wifi, zatím ignoruj
#define STASSID "xxxxxxxx"
#define STAPSK "xxxxxxxxx"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;


void setup() {
  Serial.begin(115200);
  Wire.begin(4, 15); // započetí I2C komunikace na piny SDA=4 a SCL=15 display.begin(SSD1306_SWITCHCAPVCC, 0x3C).....4 a 15 jsou piny pro I2C komunikaci, které jsou použity pro připojení OLED displeje.
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 20);
  display.println("Ahoj svete");
  display.display();
  delay(1000);
}


void loop() {
  static unsigned long counter = 0;
  Serial.print("Hodnota čítače: ");
  Serial.println(counter++);
  delay(500);
}
  // --- Připojení k WiFi (ponecháno pouze pro ukázku, zakomentováno) ---
  // WiFi.mode(WIFI_STA);
  // WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  // }
  // ---------------------------------------------------------------
