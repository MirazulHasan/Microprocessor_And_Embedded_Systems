#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <ESP8266WiFi.h>

const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo gasKnob;
int buzzerPin = 10;
int gasSensorPin = A0;
int tempSensorPin = A1;
int weightDataPin = 2;
int weightClockPin = 3;
int weightThreshold = 12000; // in grams, 12 kg
int gasThreshold = 500; // adjust this value based on sensor calibration
int tempThreshold = 100; // in degrees Celsius

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  gasKnob.attach(9);
  pinMode(buzzerPin, OUTPUT);
  pinMode(weightDataPin, INPUT);
  pinMode(weightClockPin, OUTPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  float temp = readTemperature();
  float gasLevel = readGasLevel();
  int weight = readWeight();

  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print(" C");
  lcd.setCursor(0, 1);
  lcd.print("Gas: ");
  lcd.print(gasLevel);
  lcd.print(" ppm");
  
  if (temp > tempThreshold) {
    triggerAlarm();
    turnOffGas();
    sendNotification("High temperature detected!");
  }

  if (gasLevel > gasThreshold) {
    triggerAlarm();
    turnOffGas();
    sendNotification("Gas leakage detected!");
  }

  if (weight < weightThreshold) {
    sendNotification("Low gas level detected!");
  }

  delay(1000); // Adjust delay as needed for sampling rate
}

float readTemperature() {
  int rawValue = analogRead(tempSensorPin);
  float voltage = rawValue * (5.0 / 1023.0);
  return (voltage - 0.5) * 100; // LM35 sensor has a linear output of 10mV/°C
}

float readGasLevel() {
  return analogRead(gasSensorPin);
}

int readWeight() {
  long reading = 0;
  for (int i = 0; i < 10; ++i) {
    reading += (long)readRawWeight();
  }
  reading /= 10;
  return reading;
}

long readRawWeight() {
  long reading;
  digitalWrite(weightClockPin, HIGH);
  delayMicroseconds(100);
  digitalWrite(weightClockPin, LOW);
  delayMicroseconds(100);
  reading = shiftIn(weightDataPin, weightClockPin, MSBFIRST);
  reading |= shiftIn(weightDataPin, weightClockPin, MSBFIRST) << 8;
  reading |= shiftIn(weightDataPin, weightClockPin, MSBFIRST) << 16;
  reading |= shiftIn(weightDataPin, weightClockPin, MSBFIRST) << 24;
  digitalWrite(weightClockPin, HIGH);
  reading ^= 0x800000;
  return reading;
}

void triggerAlarm() {
  tone(buzzerPin, 1000);
  delay(1000); // Adjust duration of alarm sound
  noTone(buzzerPin);
}

void turnOffGas() {
  gasKnob.write(90); // Adjust angle as needed to turn off the gas cylinder knob
}

void sendNotification(String message) {
  // Code to send notification to smartphone using Wi-Fi connection
}
