#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include "SparkFun_SGP30_Arduino_Library.h"
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

#define RXD2 16
#define TXD2 17

boolean odczytajDanePMS(Stream *s);

struct DanePMS5003 {
  uint16_t dlugoscRamki;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t czasteczki_03um, czasteczki_05um, czasteczki_10um, czasteczki_25um, czasteczki_50um, czasteczki_100um;
  uint16_t nieuzywane;
  uint16_t sumaKontrolna;
};

DanePMS5003 dane;

SGP30 czujnikSGP30;
Adafruit_BMP280 bmp;
TinyGPSPlus gps;
SoftwareSerial gpsSerial(4, 3); 

#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS 10

unsigned long ostatniOdczyt = 0;
const unsigned long interwalOdczytu = 5000;

void setup() {
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.begin(115200);
  gpsSerial.begin(4800);
  
  Serial.println(F("Uruchamianie systemu..."));
  Wire.begin();

  if (!czujnikSGP30.begin()) {
    Serial.println("Nie wykryto SGP30. Sprawdź połączenia.");
    while (1);
  }

  czujnikSGP30.initAirQuality();

  if (!bmp.begin(0x76)) {
    Serial.println(F("Nie znaleziono czujnika BMP280, sprawdź okablowanie!"));
    while (1) delay(10);
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
}

void loop() {
  unsigned long aktualnyCzas = millis();
  if (aktualnyCzas - ostatniOdczyt >= interwalOdczytu) {
    ostatniOdczyt = aktualnyCzas;
    
    if (odczytajDanePMS(&Serial1)) {
      Serial.println("\n---------------------------------------");
      Serial.println("Stężenie pyłu (standardowe)");
      Serial.print("PM 1.0: "); Serial.print(dane.pm10_standard);
      Serial.print("\tPM 2.5: "); Serial.print(dane.pm25_standard);
      Serial.print("\tPM 10: "); Serial.println(dane.pm100_standard);
      Serial.println("---------------------------------------");
      Serial.println("Stężenie pyłu (środowiskowe)");
      Serial.print("PM 1.0: "); Serial.print(dane.pm10_env);
      Serial.print("\tPM 2.5: "); Serial.print(dane.pm25_env);
      Serial.print("\tPM 10: "); Serial.println(dane.pm100_env);
      Serial.println("---------------------------------------");
      Serial.print("Cząsteczki > 0.3µm: "); Serial.println(dane.czasteczki_03um);
      Serial.print("Cząsteczki > 0.5µm: "); Serial.println(dane.czasteczki_05um);
      Serial.print("Cząsteczki > 1.0µm: "); Serial.println(dane.czasteczki_10um);
      Serial.print("Cząsteczki > 2.5µm: "); Serial.println(dane.czasteczki_25um);
      Serial.print("Cząsteczki > 5.0µm: "); Serial.println(dane.czasteczki_50um);
      Serial.print("Cząsteczki > 10.0µm: "); Serial.println(dane.czasteczki_100um);
      Serial.println("---------------------------------------");
    }

    Serial.print("Temperatura: ");
    Serial.print(bmp.readTemperature());
    Serial.println(" °C");

    Serial.print("Ciśnienie: ");
    Serial.print(bmp.readPressure());
    Serial.println(" Pa");

    Serial.print("Wysokość: ");
    Serial.print(bmp.readAltitude(1011.9));
    Serial.println(" m");

    Serial.println();
    
    czujnikSGP30.measureAirQuality();
    Serial.print("CO2: ");
    Serial.print(czujnikSGP30.CO2);
    Serial.print(" ppm\tTVOC: ");
    Serial.print(czujnikSGP30.TVOC);
    Serial.println(" ppb");
  }

  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isValid()) {
    Serial.print("Lokalizacja GPS: ");
    Serial.print("Szerokość: "); Serial.print(gps.location.lat(), 6);
    Serial.print(", Długość: "); Serial.println(gps.location.lng(), 6);
  } else {
    Serial.println("Brak danych GPS");
  }

  delay(1000);
}

boolean odczytajDanePMS(Stream *s) {
  if (!s->available()) {
    return false;
  }

  if (s->peek() != 0x42) {
    s->read();
    return false;
  }

  if (s->available() < 32) {
    return false;
  }

  uint8_t bufor[32];
  uint16_t suma = 0;
  s->readBytes(bufor, 32);

  for (uint8_t i = 0; i < 30; i++) {
    suma += bufor[i];
  }

  uint16_t bufor_u16[15];
  for (uint8_t i = 0; i < 15; i++) {
    bufor_u16[i] = bufor[2 + i * 2 + 1];
    bufor_u16[i] += (bufor[2 + i * 2] << 8);
  }

  memcpy((void *)&dane, (void *)bufor_u16, 30);

  if (suma != dane.sumaKontrolna) {
    Serial.println("Błąd sumy kontrolnej");
    return false;
  }

  return true;
}
