# Camper-Gaswaage

Die Camper-Gaswaage ist ein Open-Source-Projekt zur digitalen Überwachung des Füllstands von handelsüblichen Gasflaschen (z. B. 5 kg oder 11 kg). Das System misst das Gewicht über eine 50-kg-Wägezelle, berechnet anhand des Leergewichts (Tara) den reinen Gasinhalt und gibt diesen in Kilogramm sowie als Prozentwert aus.

Das System arbeitet völlig autark: Der Mikrocontroller spannt ein eigenes WLAN-Netzwerk auf. Es ist keine Internetverbindung oder externe Cloud-Anbindung notwendig.

## Features
* **Lokaler Access Point:** Der ESP8266 erstellt ein eigenes WLAN.
* **Webinterface:** Ablesen der Daten über den Browser auf dem Smartphone oder Notebook.
* **OLED-Display:** Direkte Anzeige der Werte am Einbauort.
* **Persistenter Speicher:** Das Leergewicht der Flasche wird über das Webinterface eingegeben und überlebt im EEPROM auch einen Stromausfall.

## Benötigte Hardware
* 1x Wemos D1 Mini (oder ein anderer ESP8266-Mikrocontroller)
* 1x Wägezelle CZL601 (Single-Point, Nennlast 50 kg)
* 1x HX711 Wägezellen-Verstärker-Modul
* 1x 0.96 Zoll OLED-Display (I2C, SSD1306)
* 2x Holz- oder Metallplatten für den Bau der Waagen-Mechanik

## Verkabelung (Pinout)

**HX711 an Wemos D1 Mini**
* VCC -> 5V
* GND -> GND
* DT  -> D6
* SCK -> D5

**Wägezelle (CZL601) an HX711**
* Rot   (Excitation+) -> E+
* Schwarz (Excitation-) -> E-
* Weiß  (Signal-)     -> A-
* Grün  (Signal+)     -> A+

**OLED an Wemos D1 Mini**
* VCC -> 3.3V
* GND -> GND
* SDA -> D2
* SCL -> D1

## Software und Bibliotheken
Das Projekt wird mit der Arduino IDE auf den Wemos D1 Mini geflasht. Folgende Bibliotheken müssen über den Bibliotheksverwalter installiert werden:
* `HX711 Arduino Library` (von Bogdan Necula)
* `Adafruit GFX Library`
* `Adafruit SSD1306`

## Inbetriebnahme & Kalibrierung
1. **Kalibrierung:** Vor dem produktiven Einsatz muss der Wert `CALIBRATION_FACTOR` im Code angepasst werden. Lade ein einfaches HX711-Kalibrierungs-Skript auf den ESP, lege ein bekanntes Gewicht auf die Waage und ermittle deinen individuellen Faktor. Trage diesen in den Sketch ein.
2. **Flashen:** Lade den finalen Sketch auf den Wemos D1 Mini.
3. **WLAN verbinden:** Verbinde dein Endgerät mit dem neuen WLAN.
   * **SSID:** Gaswaage
   * **Passwort:** passwort
4. **Webinterface aufrufen:** Öffne den Browser und navigiere zu `http://192.168.8.6`.
5. **Tara einstellen:** Lies das Leergewicht deiner Gasflasche ab (eingestanzt am Kragen) und speichere es über das Webinterface.

6. ## Autor

**Fred Fiedler** Fotograf und Fernwehgeplagter  
GitHub: [Casaluifred](https://github.com/Casaluifred) 
Web: [XTramp](https://www.xtramp.de)
