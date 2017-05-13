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
#define DS18020_PIN       14

const char *ssid     = "ESP_FREEZER";
const char *password = "j989mik7!";

volatile float     currentTemperature(0);
OneWire            oneWire(DS18020_PIN);
DallasTemperature  sensor(&oneWire);
Adafruit_SSD1306   display(DISPLAY_RESET_PIN);
ESP8266WebServer server(80);
void handleRoot();

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
}

void loop() {
  server.handleClient();
  
  // put your main code here, to run repeatedly:
  sensor.requestTemperatures();
  currentTemperature = sensor.getTempCByIndex(0);
  Serial.print("Temperatures: ");
  Serial.println(currentTemperature);
  delay(500);

  display.clearDisplay();
  display.fillCircle(display.width() / 2, display.height() / 2, 31, WHITE);
  display.setTextColor(BLACK, WHITE);
  display.setTextSize(2);
  display.setCursor(34, 24);
  display.print(currentTemperature);
  display.display();
}

void handleRoot() {
  char tempStr[8];
  char buffer[64];
  memset(buffer, 0, 64);
  dtostrf(currentTemperature, 4, 2, tempStr);
  sprintf(buffer, "Temperature: %s C", tempStr);
  server.send(200, "text/html", buffer);
}
