#include "Arduino.h"
#include <Wire.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "PCF8574.h"
#include "esp_task_wdt.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// TEST_MODE by default disabled to restore original behaviour
#define TEST_MODE
#define TEST_DEMO_INTERVAL 500 // interval v ms mezi spínáním

#ifndef STASSID
#define STASSID "rete.cz-Prichy"
#define STAPSK "adamekprichy1"
#endif

#define name "sonoff_00"

                           

const char* ssid = STASSID;           // wifi ssid name
const char* password = STAPSK;        // wifi password

int refresh_time_display = 1000;             
bool led_status = true;
int lastExecutedMillis;

bool online;
bool switchState;
int switchState_in;
bool status1;
bool status2;
int value1;
int value2;
int value3;
int value4;

// Water Sensor pins
#define TRIG 12
#define ECHO 13

// Kalibrační konstanty nádrže (upravte po instalaci podle skutečné nádrže)
// NOTE: watchdog initialization intentionally performed inside setup();
// accidental global-scope calls were removed to avoid compiler errors.
#define TANK_EMPTY_CM 150     // Vzdálenost k prázdné nádrži (cm)
#define TANK_FULL_CM 20       // Vzdálenost k plné nádrži (cm)

// U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0,  15, 4, U8X8_PIN_NONE);//SCL IO15  SDA  IO4

// Set i2c address
PCF8574 pcf8574(0x24);
PCF8574 pcf8574_input(0x22);


int layer = 0;

WiFiServer server(80);

// Funkce pro výpočet procent naplnění nádrže
int calculateWaterPercentage(int distance) {
  if (distance < 0) return -1;                  // Chyba měření
  if (distance <= TANK_FULL_CM) return 100;    // Plná nádrž
  if (distance >= TANK_EMPTY_CM) return 0;     // Prázdná nádrž
  
  // Výpočet procent (invertované - menší vzdálenost = více vody)
  int percentage = map(distance, TANK_FULL_CM, TANK_EMPTY_CM, 100, 0);
  return constrain(percentage, 0, 100);
}

// Funkce pro získání textového stavu nádrže
String getTankStatus(int percentage) {
  if (percentage < 0) return "CHYBA";
  if (percentage >= 80) return "PLNA";
  if (percentage >= 50) return "POLOPLNA";
  if (percentage >= 20) return "NIZKA";
  return "PRAZDNA";
}

void handleRequest(WiFiClient client, String request) {
  if (request.indexOf("GET /getdata") != -1) {
    online = request.indexOf("online=") != -1 ? request.substring(request.indexOf("online=") + 7, request.indexOf("&", request.indexOf("online="))).toInt() : false;
    switchState_in = request.indexOf("switchState=") != -1 ? request.substring(request.indexOf("switchState=") + 12, request.indexOf("&", request.indexOf("switchState="))).toInt() : 0;
    status1 = request.indexOf("status1=") != -1 ? request.substring(request.indexOf("status1=") + 8, request.indexOf("&", request.indexOf("status1="))).toInt() : false;
    status2 = request.indexOf("status2=") != -1 ? request.substring(request.indexOf("status2=") + 8, request.indexOf("&", request.indexOf("status2="))).toInt() : false;
    value1 = request.indexOf("value1=") != -1 ? request.substring(request.indexOf("value1=") + 7, request.indexOf("&", request.indexOf("value1="))).toInt() : 0;
    value2 = request.indexOf("value2=") != -1 ? request.substring(request.indexOf("value2=") + 7, request.indexOf("&", request.indexOf("value2="))).toInt() : 0;
    value3 = request.indexOf("value3=") != -1 ? request.substring(request.indexOf("value3=") + 7, request.indexOf("&", request.indexOf("value3="))).toInt() : 0;
    value4 = request.indexOf("value4=") != -1 ? request.substring(request.indexOf("value4=") + 7, request.indexOf("&", request.indexOf("value4="))).toInt() : 0;

    // Odstraněno: měření přes TRIG/ECHO
    // Zobrazíme pouze poslední naměřené hodnoty
    extern int distance;
    extern int waterPercentage;
    String response = "<html><body>";
    response += "<h1>Zavlazovaci system:</h1>";
    response += "<h2>Nadrz na vodu:</h2>";
    if (distance > 0) {
      response += "<p><b>Vzdalenost k hladine: " + String(distance) + " cm</b></p>";
      response += "<p><b>Naplneni nadrze: " + String(waterPercentage) + " %</b></p>";
      response += "<p><b>Stav nadrze: " + getTankStatus(waterPercentage) + "</b></p>";
    } else {
      response += "<p><b>Chyba mereni senzoru!</b></p>";
    }
    response += "<hr>";
    response += "<h2>Stavove data:</h2>";
    response += "<p>online = " + String(online) + "</p>";
    response += "<p>switchState = " + String(switchState) + "</p>";
    response += "<p>status1 = " + String(status1) + "</p>";
    response += "<p>status2 = " + String(status2) + "</p>";
    response += "<p>value1 = " + String(value1) + "</p>";
    response += "<p>value2 = " + String(value2) + "</p>";
    response += "<p>value3 = " + String(value3) + "</p>";
    response += "<p>value4 = " + String(value4) + "</p>";

    int32_t rssi = WiFi.RSSI();
    response += "<hr><p>Signal strength (RSSI): " + String(rssi) + " dBm</p>"; 
    response += "</body></html>";

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println("");
    client.println(response);
  } else {
    client.println("HTTP/1.1 404 Not Found");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println("");
    client.println("Page not found");
  }
}



void setup(void) {
  Serial.begin(115200);
  Wire.begin(4, 15); // SDA=4, SCL=15
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Sonoff Zavlazovani");
  display.display();
  delay(1000);
  
  // Inicializace WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  pcf8574.pinMode(P0, OUTPUT);
  pcf8574.pinMode(P1, OUTPUT);
  pcf8574.pinMode(P2, OUTPUT);
  pcf8574.pinMode(P3, OUTPUT);
  pcf8574.pinMode(P4, OUTPUT);
  pcf8574.pinMode(P5, OUTPUT);


  pcf8574.digitalWrite(P0, HIGH);
  pcf8574.digitalWrite(P1, HIGH);
  pcf8574.digitalWrite(P2, HIGH);
  pcf8574.digitalWrite(P3, HIGH);
  pcf8574.digitalWrite(P4, HIGH);
  pcf8574.digitalWrite(P5, HIGH);

  Serial.print("Init pcf8574...");
  if (pcf8574.begin()) {
    Serial.println("OK");
  } else {
    Serial.println("KO");
  }


  pcf8574_input.pinMode(P0, INPUT);
  pcf8574_input.pinMode(P1, INPUT);
  pcf8574_input.pinMode(P2, INPUT);
  pcf8574_input.pinMode(P3, INPUT);
  pcf8574_input.pinMode(P4, INPUT);
  pcf8574_input.pinMode(P5, INPUT);

  bool pcf_input_ok = pcf8574_input.begin();
  if (pcf_input_ok) {
    Serial.println("PCF8574_input OK");
  } else {
    Serial.println("PCF8574_input KO");
  }

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("WiFi pripojeni:");
  display.println(ssid);
  display.display();
  delay(1000);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".......");
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Pripojuji WiFi...");
    display.println("Cekam na sit");
    display.display();
  }

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("WiFi: PRIPOJENO");
  display.print("IP: ");
  display.println(WiFi.localIP());
  int32_t rssi = WiFi.RSSI();
  display.print("Signal: ");
  display.print(rssi);
  display.println(" dBm");
  display.display();
  delay(2000);
  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (!MDNS.begin(name)) {
    Serial.println("Error setting up MDNS responder!");
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("mDNS: CHYBA");
    display.display();
    delay(1000);
    while (1) { delay(1000); }
  }
  Serial.println("mDNS responder started");

  server.begin();
  Serial.println("TCP server started");

  MDNS.addService("http", "tcp", 80);

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Server: SPUSTEN");
  display.println("mDNS: OK");
  display.println("HTTP: port 80");
  display.println("Watchdog: OK");
  display.display();
  delay(1500);

  esp_task_wdt_config_t twdt_config = {
      .timeout_ms = 8000,
      .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
      .trigger_panic = true
  };
  esp_task_wdt_init(&twdt_config);
  esp_task_wdt_add(NULL);

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("SYSTEM PRIPRAVEN");
  display.println("Senzor: AJ-SR04M");
  display.println("Displej: SSD1306");
  display.println("Spoustim...");
  display.display();
  delay(1000);

  // Nastavení režimu pinů pro ultrazvukový senzor
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  digitalWrite(TRIG, LOW); // TRIG na začátku v LOW

}

int distance = 0;
int waterPercentage = 0;

void loop() {
  delay(1000);
  //pcf8574.digitalWrite(P0, HIGH);
  Serial.println("P0 LOW");
  delay(1000);
 // pcf8574.digitalWrite(P0, LOW);
  Serial.println("P0 HIGH");
  // ...existing code... (běžný provoz)
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Restarting...");
    ESP.restart();
  }

  esp_task_wdt_reset();  // Obnovení watchdogu

  //  měření ultrazvukem
  
  if (millis() - lastExecutedMillis >= refresh_time_display) {
    // Spuštění ultrazvukového měření
    digitalWrite(TRIG, LOW);
    delayMicroseconds(100);
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(300);
    digitalWrite(TRIG, LOW);

    // Měření času ozvěny a výpočet vzdálenosti
    long duration = pulseIn(ECHO, HIGH, 30000); // Timeout 30ms

    if (duration > 0) {
      distance = duration * 0.034 / 2; // Převod na cm (rychlost zvuku 340 m/s)
    } else {
      distance = -1; // Chyba měření
    }

    // Výpočet procent naplnění
    waterPercentage = calculateWaterPercentage(distance);
    String tankStatus = getTankStatus(waterPercentage);

    // Výpis na displej (Adafruit_SSD1306)
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    if (waterPercentage >= 0) {
      display.print("Nadrz: ");
      display.print(waterPercentage);
      display.println("%");
      display.print("Stav: ");
      display.println(tankStatus);
    } else {
      display.println("CHYBA SENZORU");
    }
    display.setTextSize(2);
    display.setCursor(0, 32);
    if (distance > 0) {
      display.print(distance);
      display.println(" cm");
    } else {
      display.println("---");
    }
    display.display();

    lastExecutedMillis = millis();
  }
  

  WiFiClient client = server.available();
  if (client) {
    unsigned long startMillis = millis();
    while (client.connected() && !client.available()) {
      if (millis() - startMillis >= 5000) {
        Serial.println("Timeout - no data received");
        break;
      }
      delay(1);
    }

    if (client.connected() && client.available()) {
      String request = client.readStringUntil('\r');
      if (!request.isEmpty()) {
        handleRequest(client, request);
      }
    }

    if (!client.connected()) {
      client.stop();
    }
  }

  if (switchState_in == 0) {switchState = false;}
  if (switchState_in == 1) {switchState = true; }

  //pcf8574.digitalWrite(P1, switchState ? LOW : HIGH);
}
