#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <gfxfont.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define DISPLAY_RESET_PIN 15
#define SERIAL_RATE       9600
#define FREEZER_PIN       16
#define DS18020_PIN       14
#define ENCODER_LEFT_PIN  12
#define ENCODER_RIGHT_PIN 13
#define INCREASE          {setupTemperature += TEMPERATURE_STEP;}
#define DECREASE          {setupTemperature -= TEMPERATURE_STEP;}
#define TEMPERATURE_STEP  0.1f
#define RETENTION         1.0f   // 1 degree
#define UNREACHABLE       -274.0f
#define LOW_LIMIT         -5.0f

const char *ssid     = "ESP_FREEZER";
const char *password = "j989mik7!";

volatile float     setupTemperature(UNREACHABLE);
volatile float     currentTemperature(0);
OneWire            oneWire(DS18020_PIN);
DallasTemperature  sensor(&oneWire);
Adafruit_SSD1306   display(DISPLAY_RESET_PIN);
ESP8266WebServer server(80);

void handleRoot();
void handleInterrupted();

void setup() {
  delay(1000);

  // put your setup code here, to run once:
  Serial.begin(SERIAL_RATE);
  sensor.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();

  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");

  // setup interrupt for EC12 and Freezer output
  pinMode(ENCODER_LEFT_PIN, INPUT_PULLUP);
  pinMode(ENCODER_RIGHT_PIN, INPUT_PULLUP);
  pinMode(FREEZER_PIN, OUTPUT);
  digitalWrite(FREEZER_PIN, LOW);
  attachInterrupt(digitalPinToInterrupt(ENCODER_LEFT_PIN), handleInterrupted, CHANGE);
}

void loop() {
  server.handleClient();

  // put your main code here, to run repeatedly:
  sensor.requestTemperatures();
  currentTemperature = sensor.getTempCByIndex(0);
  if (setupTemperature <= UNREACHABLE) {
    setupTemperature = currentTemperature;
  }
  if (currentTemperature >= setupTemperature) {
    digitalWrite(FREEZER_PIN, HIGH);
  } else if (currentTemperature <= setupTemperature - RETENTION) {
    digitalWrite(FREEZER_PIN, LOW);
  }
  
  Serial.print("Temperatures: ");
  Serial.println(currentTemperature);
  delay(500);

  {
    display.clearDisplay();
    {
      // Current temperature
      display.fillCircle(display.width() / 2, display.height() / 2, 31, WHITE);
      display.setTextColor(BLACK, WHITE);
      display.setTextSize(2);
      display.setCursor(34, 24);
      display.print(currentTemperature);
    }

    {
      // Setup tempereture
      display.setTextColor(WHITE);
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print(setupTemperature);
    }
    display.display();
  }
}

void handleInterrupted() {
  byte leftPin  = digitalRead(ENCODER_LEFT_PIN);
  byte rightPin = digitalRead(ENCODER_RIGHT_PIN);
  if ((leftPin == HIGH && rightPin == HIGH) || (leftPin == LOW && rightPin == LOW)) {
    DECREASE; // - <- +
  } else {
    INCREASE; // - -> +
  }
  setupTemperature = setupTemperature < LOW_LIMIT ? LOW_LIMIT : setupTemperature;
}

void handleRoot() {
  char tempStr[8];
  char buffer[64];
  memset(buffer, 0, 64);
  dtostrf(currentTemperature, 4, 2, tempStr);
  sprintf(buffer, "Temperature: %s C", tempStr);
  server.send(200, "text/html", buffer);
}
