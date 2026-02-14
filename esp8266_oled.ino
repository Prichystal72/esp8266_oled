/*
  Projekt: OLED displej s ESP8266
  Autor: (doplňte své jméno)
  
  Tento program je určený pro čip ESP8266 (například NodeMCU nebo Wemos D1 mini) a OLED displej 128x64 pixelů s čipem SSD1306.

  Jak postupovat v Arduino IDE:
  --------------------------------
  1. V menu "Nástroje > Deska" vyberte: "NodeMCU 1.0 (ESP-12E Module)" nebo jinou odpovídající ESP8266 desku.
  2. Připojte OLED displej podle popisu níže:
     - SDA (datový drát) na pin D5 (GPIO14)
     - SCL (hodinový drát) na pin D6 (GPIO12)
  3. Nahrajte knihovny:
     - Adafruit SSD1306
     - Adafruit GFX
     - Wire
  4. Vložte tento kód do Arduino IDE a nahrajte do desky.

  Poznámka: Pokud používáte jiné piny nebo jiný typ displeje, upravte nastavení v kódu.
*/

#include <Wire.h>                // Knihovna pro komunikaci mezi čipem a displejem (tzv. I2C)
#include <Adafruit_GFX.h>        // Knihovna, která umí kreslit text a obrázky na displej
#include <Adafruit_SSD1306.h>    // Knihovna pro konkrétní typ displeje, který používáme

#define SCREEN_WIDTH 128   // Šířka displeje v pixelech (tečkách)
#define SCREEN_HEIGHT 64   // Výška displeje v pixelech (tečkách)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire); // Nastavení displeje s rozměry a komunikací

#define I2C_SDA 14  // Pin číslo 14 na čipu (označený jako D5 na desce) - datový drát
#define I2C_SCL 12  // Pin číslo 12 na čipu (označený jako D6 na desce) - hodinový drát

void setup() {
  Serial.begin(115200); // Nastartujeme "sériovou linku" pro ladění (můžeme pak vidět zprávy v počítači)
  Wire.begin(I2C_SDA, I2C_SCL); // Spustíme komunikaci po drátech SDA a SCL
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);      // Velikost písma (1 = nejmenší)
  display.setTextColor(WHITE); // Barva písma (bílá, protože displej je černobílý)
  display.setCursor(0, 0);     // Začneme psát úplně vlevo nahoře
  display.println(F("Zluty radek"));      // První řádek textu, který se zobrazí
  display.println(F(""));   // Druhý řádek textu
  display.println(F("Modry radek"));     // Třetí řádek textu, který se zobrazí 
  display.setTextSize(2);      // Změníme velikost písma na větší (2 = dvojnásobná velikost) 
  display.setCursor(0, 30);    // Posuneme kurzor trochu dolů, aby se text překrýval
  display.println(F("Velikost 2"));    // Čtvrtý řádek
  display.display();  // Tímto příkazem opravdu pošleme vše na displej, aby se to ukázalo
}

void loop() {
  // Tato část by se opakovala pořád dokola, dokud je čip zapnutý.
  // Zatím tu nic není, ale sem můžeme později přidat další funkce.
}
