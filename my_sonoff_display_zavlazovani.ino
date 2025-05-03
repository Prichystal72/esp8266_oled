#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <PCF8574.h>
#include "esp_task_wdt.h"  // Přidání knihovny pro watchdog na ESP32

#ifndef STASSID
#define STASSID "rete.cz-Prichy"
#define STAPSK "adamekprichy1"
#endif

#define name "sonoff_00"

                           
#define RELAY           1                                
#define LED             2    
#define REDLED          3  

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
#define TRIG 2
#define ECHO 0

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0,  15, 4, U8X8_PIN_NONE);//SCL IO15  SDA  IO4

// Set i2c address
PCF8574 pcf8574(0x24,4,15);
PCF8574 pcf8574_input(0x22,4,15);

int layer = 0;

WiFiServer server(80);

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

    String response = "<html><body>";
    response += "<h1>Data from device:</h1>";
    response += "<p>online = " + String(online) + "</p>";
    response += "<p>switchState = " + String(switchState) + "</p>";
    response += "<p>status1 = " + String(status1) + "</p>";
    response += "<p>status2 = " + String(status2) + "</p>";
    response += "<p>value1 = " + String(value1) + "</p>";
    response += "<p>value2 = " + String(value2) + "</p>";
    response += "<p>value3 = " + String(value3) + "</p>";
    response += "<p>value4 = " + String(value4) + "</p>";

    int32_t rssi = WiFi.RSSI();
    response += "<p>Signal strength (RSSI):" + String(rssi) + "dBm </p>"; 
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
  
  u8g2.setI2CAddress(0x3C*2);
  u8g2.begin();
  u8g2.enableUTF8Print();
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

 // Nastavení pinů a jejich stavů pomocí smyčky
for (int pin = 0; pin <= 5; pin++) {
  pcf8574.pinMode(pin, OUTPUT);
  pcf8574.digitalWrite(pin, HIGH);
}

 // Nastavení pinů a jejich stavů pomocí smyčky
for (int pin = 0; pin <= 5; pin++) {
  pcf8574_input.pinMode(pin, INPUT);

}


  if (pcf8574.begin()) {
    Serial.println("PCF8574 OK");
  } else {
    Serial.println("PCF8574 KO");
  }

  if (pcf8574_input.begin()) {
    Serial.println("PCF8574_input OK");
  } else {
    Serial.println("PCF8574_input KO");
  }


  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".......");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (!MDNS.begin(name)) {
    Serial.println("Error setting up MDNS responder!");
    while (1) { delay(1000); }
  }
  Serial.println("mDNS responder started");

  server.begin();
  Serial.println("TCP server started");

  MDNS.addService("http", "tcp", 80);

  esp_task_wdt_init(8, true);  // Nastaví watchdog na 8 sekund

  //pcf8574.digitalWrite(P1,  HIGH);
}

void loop(void) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Restarting...");
    ESP.restart();
  }

  esp_task_wdt_reset();  // Obnovení watchdogu

  if (millis() - lastExecutedMillis >= refresh_time_display) {
    //digitalWrite(TRIG, LOW); 
    delayMicroseconds(2); 
    //digitalWrite(TRIG, HIGH);  
    delayMicroseconds(20); 
    //digitalWrite(TRIG, LOW);  

    int distance = pulseIn(ECHO, HIGH, 26000) / 58; 
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 15, "Stav vody:");
    u8g2.setFont(u8g2_font_fub30_tr); 

    char buffer[10];
    sprintf(buffer, "%d mm", distance);
    u8g2.drawStr(0, 60, buffer);
    u8g2.sendBuffer();

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




  pcf8574.digitalWrite(RELAY, switchState ? LOW : HIGH);
  pcf8574.digitalWrite(8, switchState ? LOW : HIGH);
  
}
