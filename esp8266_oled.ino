
// Knihovna pro komunikaci přes I2C (propojení s OLED displejem)
#include <Wire.h>
// Knihovna pro WiFi připojení (zatím není využita)
#include <WiFi.h>
// Knihovna pro grafické funkce (základ pro práci s displejem)
#include <Adafruit_GFX.h>
// Knihovna pro ovládání OLED displeje SSD1306
#include <Adafruit_SSD1306.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// inicializace displeje s použitím I2C (SDA=12 (D6), SCL=14 (D5))
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


// --- Nastavení WiFi (zatím není využito, pouze pro ukázku) ---
#ifndef STASSID  // Pokud není definováno, nastaví se výchozí hodnoty
#define STASSID "xxxxxxxx"   // zde zadejte název vaší WiFi sítě
#define STAPSK "xxxxxxxxx"   // zde zadejte heslo k WiFi
#endif

const char* ssid = STASSID;      // proměnná pro název WiFi
const char* password = STAPSK;   // proměnná pro heslo k WiFi


// Funkce setup() se spustí pouze jednou po startu nebo resetu zařízení
void setup() {
  Serial.begin(115200); // Nastavení rychlosti sériové komunikace pro ladění (výpisy do PC)
  // Inicializace I2C sběrnice na pinech SDA=12 a SCL=14 (pro OLED displej)
  Wire.begin(12, 14);
  // Inicializace OLED displeje, adresa 0x3C je běžná pro tento typ displeje
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay(); // Vymaže obsah displeje
  display.setTextSize(2); // Nastaví velikost písma (2x větší než základní)
  display.setTextColor(SSD1306_WHITE); // Nastaví barvu textu (bílá)
  display.setCursor(0, 20); // Nastaví pozici kurzoru (x=0, y=20)
  display.println("Ahoj svete"); // Vypíše text na displej
  display.display(); // Aktualizuje displej, aby se změny projevily
  delay(1000); // Počká 1 sekundu
}


// Funkce loop() se opakuje stále dokola, dokud je zařízení zapnuté
void loop() {
  static unsigned long counter = 0; // Statická proměnná uchovává hodnotu mezi průchody
  Serial.print("Hodnota čítače: "); // Vypíše text do sériového monitoru
  Serial.println(counter++);         // Vypíše hodnotu čítače a zvýší ji o 1
  delay(500); // Počká 0,5 sekundy
}

// --- Připojení k WiFi (ponecháno pouze pro ukázku, zakomentováno) ---
// Pokud byste chtěli připojit zařízení k WiFi, odkomentujte následující řádky a zadejte správné údaje:
// WiFi.mode(WIFI_STA);              // Nastaví režim WiFi na stanici (klient)
// WiFi.begin(ssid, password);       // Zahájí připojení k WiFi síti
// while (WiFi.status() != WL_CONNECTED) { // Čeká, dokud není připojeno
//   delay(500);
// }
// ---------------------------------------------------------------
