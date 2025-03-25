#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include "SparkFun_SGP30_Arduino_Library.h" 

#define RXD2 16 // To sensor TXD
#define TXD2 17 // To sensor RXD

// Deklaracja funkcji
boolean readPMSdata(Stream *s);
 
struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};
 
struct pms5003data data;


 SGP30 mySensor; //create an object of the SGP30 class
 
#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)
unsigned long lastRead = 0; // Zmienna śledząca czas od ostatniego odczytu
const unsigned long readInterval = 5000;
Adafruit_BMP280 bmp; // I2C
//Adafruit_BMP280 bmp(BMP_CS); // hardware SPI
//Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

void setup() { 
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.begin(115200);
  while ( !Serial ) delay(100);   // wait for native usb
  Serial.println(F("BMP280 test"));
  unsigned status;  
  Wire.begin();
  //Initialize sensor
  if (mySensor.begin() == false) 
{
    Serial.println("No SGP30 Detected. Check connections.");
    while (1);
  }
  //Initializes sensor for air quality readings
  //measureAirQuality should be called in one second increments after a call to initAirQuality
  mySensor.initAirQuality();
  //status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);
  status = bmp.begin(0x76);
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void loop() {
  unsigned long currentMillis = millis();
      if (currentMillis - lastRead >= readInterval) {
    lastRead = currentMillis;
    if (readPMSdata(&Serial1)) {
    // reading data was successful!
    Serial.println();
    Serial.println("---------------------------------------");
    Serial.println("Concentration Units (standard)");
    Serial.print("PM 1.0: "); Serial.print(data.pm10_standard);
    Serial.print("\t\tPM 2.5: "); Serial.print(data.pm25_standard);
    Serial.print("\t\tPM 10: "); Serial.println(data.pm100_standard);
    Serial.println("---------------------------------------");
    Serial.println("Concentration Units (environmental)");
    Serial.print("PM 1.0: "); Serial.print(data.pm10_env);
    Serial.print("\t\tPM 2.5: "); Serial.print(data.pm25_env);
    Serial.print("\t\tPM 10: "); Serial.println(data.pm100_env);
    Serial.println("---------------------------------------");
    Serial.print("Cząsteczki > 0.3um / 0.1L powietrza:"); Serial.println(data.particles_03um);
    Serial.print("Cząsteczki > 0.5um / 0.1L powietrza:"); Serial.println(data.particles_05um);
    Serial.print("Cząsteczki > 1.0um / 0.1L powietrza:"); Serial.println(data.particles_10um);
    Serial.print("Cząsteczki > 2.5um / 0.1L powietrza:"); Serial.println(data.particles_25um);
    Serial.print("Cząsteczki > 5.0um / 0.1L powietrza:"); Serial.println(data.particles_50um);
    Serial.print("Cząsteczki > 10.0 um / 0.1L powietrza:"); Serial.println(data.particles_100um);
    Serial.println("---------------------------------------");
  }
    Serial.print(F("Temperatura = "));
    Serial.print(bmp.readTemperature());
    Serial.println(" *C");

    Serial.print(F("Ciśnienie = "));
    Serial.print(bmp.readPressure());
    Serial.println(" Pa");

    Serial.print(F("Wysokość ok. = "));
    Serial.print(bmp.readAltitude(1011.9)); /* Adjusted to local forecast! */
    Serial.println(" m");

    Serial.println();
}
{
  //First fifteen readings will be
  //CO2: 400 ppm  TVOC: 0 ppb
  delay(1000); //Wait 1 second
  //measure CO2 and TVOC levels
  mySensor.measureAirQuality();
  Serial.print("CO2: ");
  Serial.print(mySensor.CO2);
  Serial.print(" ppm\tTVOC: ");
  Serial.print(mySensor.TVOC);
  Serial.println(" ppb");
}
}
boolean readPMSdata(Stream *s) {
  if (! s->available()) {
    return false;
  }
 
  // Read a byte at a time until we get to the special '0x42' start-byte
  if (s->peek() != 0x42) {
    s->read();
    return false;
  }
 
  // Now read all 32 bytes
  if (s->available() < 32) {
    return false;
  }
 
  uint8_t buffer[32];
  uint16_t sum = 0;
  s->readBytes(buffer, 32);
 

  for (uint8_t i = 0; i < 30; i++) {
    sum += buffer[i];
  }
 
  uint16_t buffer_u16[15];
  for (uint8_t i = 0; i < 15; i++) {
    buffer_u16[i] = buffer[2 + i * 2 + 1];
    buffer_u16[i] += (buffer[2 + i * 2] << 8);
  }
 
  memcpy((void *)&data, (void *)buffer_u16, 30);
 
  if (sum != data.checksum) {
    Serial.println("Checksum failure");
    return false;
  }

  return true;
}