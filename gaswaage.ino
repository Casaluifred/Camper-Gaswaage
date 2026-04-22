/*
 * PROJEKT: WLAN-Gaswaage
 * V2.4 (c) Fred Fiedler 2021
 * HARDWARE: Wemos D1 Mini (ESP8266), HX711, OLED 0.96" (I2C)
 * SENSOR: CZL601 Wägezelle (50 kg Nennlast, Parallel Beam)
 * * VERSCHALTUNG:
 * HX711 Modul:
 * - VCC  -> Wemos 5V
 * - GND  -> Wemos GND
 * - DT   -> Wemos D6
 * - SCK  -> Wemos D5
 * * Anschluss CZL601 an HX711:
 * - Rot   (Excitation+) -> E+
 * - Schwarz (Excitation-) -> E-
 * - Weiß  (Signal-)     -> A-
 * - Grün  (Signal+)     -> A+
 * * OLED Display (I2C):
 * - VCC  -> Wemos 3.3V
 * - GND  -> Wemos GND
 * - SDA  -> Wemos D2
 * - SCL  -> Wemos D1
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HX711.h>

// --- Pin-Definitionen ---
const int HX711_DT = D6;
const int HX711_SCK = D5;

// --- OLED-Einstellungen ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Konstanten & Variablen ---
const float MAX_GAS_WEIGHT = 11.0; // hier das Füllgewicht der Gasflasche eintragen
const float CALIBRATION_FACTOR = -21500.0; // Kalibrierungswert für CZL601

HX711 scale;
ESP8266WebServer server(80);

float emptyWeight = 0.0;    // Gespeichertes Leergewicht
float currentWeight = 0.0;  // Aktuelle Messung (Brutto)
float netWeight = 0.0;      // Berechnetes Gasgewicht
int fillPercentage = 0;     // Füllstand in %

unsigned long previousMillis = 0;
const long interval = 2000; 

// --- WLAN-Einstellungen ---
const char* ssid = "Gaswaage";
const char* password = "passwort"; // Standard-Passwort
IPAddress local_IP(192, 168, 8, 6); // IP-Adresse der Gaswaage
IPAddress gateway(192, 168, 8, 6);
IPAddress subnet(255, 255, 255, 0);

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Gaswaage</title>";
  html += "<style>body{font-family:Arial,sans-serif; margin:20px; text-align:center;} ";
  html += "input[type='text']{padding:10px; font-size:16px; width:80px;} ";
  html += "input[type='submit']{padding:10px 20px; font-size:16px; background-color:#4CAF50; color:white; border:none; border-radius:5px;} ";
  html += ".data{font-size:28px; font-weight:bold; color:#2c3e50;}</style></head><body>";
  
  html += "<h1>Gasflaschen-Status</h1>";
  html += "<p>Netto-Gasinhalt:</p>";
  html += "<p class='data'>" + String(netWeight, 2) + " kg</p>";
  html += "<p>Füllstand:</p>";
  html += "<p class='data'>" + String(fillPercentage) + " %</p>";
  
  html += "<hr><h2>Einstellungen</h2>";
  html += "<form action='/save' method='GET'>";
  html += "<p>Tara der Flasche (kg):</p>";
  html += "<input type='text' name='tara' value='" + String(emptyWeight, 2) + "' required><br><br>";
  html += "<input type='submit' value='Speichern'>";
  html += "</form>";
  html += "<p><small>Das Leergewicht findest Du eingestanzt am Flaschenkragen.</small></p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleSave() {
  if (server.hasArg("tara")) {
    String taraStr = server.arg("tara");
    taraStr.replace(",", ".");
    emptyWeight = taraStr.toFloat();
    
    EEPROM.put(0, emptyWeight);
    EEPROM.commit();
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  
  EEPROM.begin(512);
  EEPROM.get(0, emptyWeight);
  
  // Plausibilitätscheck nach erstem Start
  if(isnan(emptyWeight) || emptyWeight < 0 || emptyWeight > 60) {
    emptyWeight = 12.5; // Typischer Default-Wert für 11kg Flaschen
  }

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Display-Fehler");
  }
  display.clearDisplay();
  display.setTextColor(WHITE);

  scale.begin(HX711_DT, HX711_SCK);
  scale.set_scale(CALIBRATION_FACTOR);

  // Access Point mit Passwort
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);
  
  server.on("/", handleRoot);
  server.on("/save", handleSave);
  server.begin();
}

void loop() {
  server.handleClient();
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    if (scale.is_ready()) {
      currentWeight = scale.get_units(5); // Mittelwert aus 5 Messungen
      
      netWeight = currentWeight - emptyWeight;
      if (netWeight < 0) netWeight = 0;
      
      fillPercentage = (netWeight / MAX_GAS_WEIGHT) * 100;
      if (fillPercentage > 100) fillPercentage = 100;
      if (fillPercentage < 0) fillPercentage = 0;

      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("Inhalt (Netto):");
      
      display.setTextSize(2);
      display.setCursor(0, 15);
      display.print(netWeight, 2);
      display.print(" kg");
      
      display.setTextSize(1);
      display.setCursor(0, 38);
      display.print("Fuellstand:");
      
      display.setTextSize(2);
      display.setCursor(0, 50);
      display.print(fillPercentage);
      display.print(" %");
      
      display.display();
    }
  }
}
