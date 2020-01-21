/*
  Tutorial: Log Sensor's Data to Blynk Server Using GPRS on TTGO ESP32 SIM800L
  Board:
  - TTGO T-Call ESP32 with SIM800L GPRS Module
    https://my.cytron.io/p-ttgo-t-call-esp32-with-sim800l-gprs-module
  
  Sensor:
  - MPL3115A2 I2C Barometric/Altitude/Temperature Sensor
    https://my.cytron.io/p-mpl3115a2-i2c-barometric-altitude-temperature-sensor
    Connection    ESP32 | MPL3115A2
                     5V - Vin
                    GND - GND
                     22 - SCL
                     21 - SDA
  
  External libraries:
  - Adafruit MPL3115A2 Library by Adafruit Version 1.2.2
  - Blynk by Volodymyr Shymanskyy Version 0.6.1
  - TinyGSM by Volodymyr Shymanskyy Version 0.7.9
  
  Created by:
  20 Jan 2020   Idris Zainal Abidin, Cytron Technologies
*/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#define TINY_GSM_MODEM_SIM800

// Default heartbeat interval for GSM is 60
// If you want override this value, uncomment and set this option:
//#define BLYNK_HEARTBEAT 30

#include <Wire.h>
#include <Adafruit_MPL3115A2.h>
#include <TinyGsmClient.h>
#include <BlynkSimpleTinyGSM.h>

#define SIM800L_RX     27
#define SIM800L_TX     26
#define SIM800L_PWRKEY 4
#define SIM800L_RST    5
#define SIM800L_POWER  23

#define BUTTON1 15
#define BUTTON2 0
#define LED_BLUE  13
#define LED_GREEN 14
#define PIEZO   25

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "YourBlynkAuthToken";

// Your GPRS credentials
// Leave empty, if missing user or pass
char apn[]  = "YourAPN";
char user[] = "";
char pass[] = "";

// Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1
TinyGsm modem(SerialAT);

Adafruit_MPL3115A2 baro = Adafruit_MPL3115A2();

float pascals = 0.0;
float altitude = 0.0;
float celsius = 0.0;
long prevMillisSensor = 0;
int intervalSensor = 2000;
long prevMillisBlynk = 0;
int intervalBlynk = 20000;
byte counter = 0;
boolean ledState = LOW;

void setup()
{
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(PIEZO, OUTPUT);
  pinMode(SIM800L_PWRKEY, OUTPUT);
  pinMode(SIM800L_RST, OUTPUT);
  pinMode(SIM800L_POWER, OUTPUT);

  digitalWrite(SIM800L_PWRKEY, LOW);
  digitalWrite(SIM800L_RST, HIGH);
  digitalWrite(SIM800L_POWER, HIGH);

  digitalWrite(LED_BLUE, LOW);
  
  // Debug console
  Serial.begin(115200);

  delay(10);

  if (!baro.begin()) {
    Serial.println("Couldn't find sensor");
  }

  // Set GSM module baud rate
  SerialAT.begin(115200, SERIAL_8N1, SIM800L_TX, SIM800L_RX);
  delay(3000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println("Initializing modem...");
  modem.restart();

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");

  Blynk.begin(auth, modem, apn, user, pass);

  digitalWrite(LED_BLUE, HIGH);

  prevMillisSensor = millis();
  prevMillisBlynk = millis();
}

void loop()
{
  Blynk.run();

  if (millis() - prevMillisSensor > intervalSensor) {
    if (!baro.begin()) {
      Serial.println("Couldn't find sensor");
    }
    else {
      pascals = baro.getPressure() / 1000;
      Serial.print(pascals);
      Serial.print(" kPa\t");
  
      altitude = baro.getAltitude();
      Serial.print(altitude);
      Serial.print(" meters\t");
  
      celsius = baro.getTemperature();
      Serial.print(celsius);
      Serial.println("*C");
    }
    
    prevMillisSensor = millis();
  }

  if (millis() - prevMillisBlynk > intervalBlynk) {

    if (counter == 1) {
      // Up data to Blynk
      Serial.println("Publish pressure data to Blynk V0");
      Blynk.virtualWrite(0, pascals);
    }
    else if (counter == 2) {
      // Up data to Blynk
      Serial.println("Publish altitude data to Blynk V1");
      Blynk.virtualWrite(1, altitude);
    }
    else if (counter == 3) {
      // Up data to Blynk
      Serial.println("Publish temperature data to Blynk V2");
      Blynk.virtualWrite(2, celsius);
      counter = 0;
    }
    counter++;

    digitalWrite(LED_BLUE, ledState);
    ledState = !ledState;
    
    prevMillisBlynk = millis();
  }
}
