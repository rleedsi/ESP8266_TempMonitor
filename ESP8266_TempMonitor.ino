// Temperature monitor using DS12B80
// Copyright (C) 2016 Richard Lee
// 2016Mar12 Created (from HydroDipThermostat) -- RL
// 2016Nov18 Remove cruft -- RL

#include <Arduino.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>

#define TEMP_HYSTERESIS 0.1

// Globals

const char ssid[] = "ssid";
const char password[] = "password";
const char host[] = "1.3.2.4";
const int httpPort = 80;

const int sleepTimeS = 30;

OneWire oneWire(13); // Pin #
DallasTemperature ts(&oneWire);
float fTemperature;
float fOffSetpoint;
float fIdleSetpoint;
float fRunSetpoint;
char szTemperature[40];
byte tsAddr[8]; // address of 18b20
uint16_t bSpUpd;

// readTemp()
// Read temperature and return value as a float in degrees Fahrenheit
// Returns -255.0 on error
// 2016Feb07 Created -- RL
float readTemp()
{
  float fRetVal = -255.0;
  ts.requestTemperatures();
  fRetVal = ts.getTempF(tsAddr);
  return(fRetVal);

} // readTemp();

// initWiFi()
void initWiFi()
{
  // Set WiFi mode to station (as opposed to AP or AP_STA)
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int i=0;
  Serial.print("Attempting to connect to network.");
  while(WiFi.status() != WL_CONNECTED && i++ < 600)
  {
    delay(100);
    Serial.print(".");
  }
  Serial.println();
  if(WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Wifi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("Wifi: Can't connect");
  }
} // initWiFi()

//******************* setup() *********************
void setup() {
  delay(1000);
  Serial.begin(115200);
  delay(1000);

  initWiFi();
  
  Serial.println("Temp Monitor");
  // Initialize the temperature sensor
  ts.begin();
  if(!ts.getAddress(tsAddr, 0))
  {
    Serial.println("No sensor found!");
  }

  // ...loop()
  static float fTemperature;
  // Process temperature
  // Take three readings, then sleep
  for(int i=0; i<3; i++)
  {
    fTemperature = readTemp();
    Serial.println(fTemperature);
    String response="";
    String post="temp=";
    post += fTemperature;
    post += "&submit=Update";
    
    WiFiClient client;

    if(!client.connect(host, httpPort))
    {
      Serial.println("Wifi connection failed!");
    }
    String postRequest =
      "POST /tempmonitor.php HTTP/1.1\r\n";
      postRequest += "Host: ";
      postRequest += host;
      postRequest += "\r\n";
      postRequest += "Accept: */*\r\n";
      postRequest += "Content-Length: ";
      postRequest += post.length();
      postRequest += "\r\n";
      postRequest += "Content-Type: application/x-www-form-urlencoded\r\n";
      postRequest += "Connection: close\r\n";
      postRequest += "\r\n";
      postRequest += post;
    client.print(postRequest);
    delay(10);
    Serial.println("Response:");
    while(client.available())
    {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    Serial.println();
    Serial.println("closing connection");

    if(fTemperature < 70.0)
    {
      Serial.println("Temp low!");
    }
    delay(20);

  } // endfor n readings

  Serial.println("Deep sleeping.");
  ESP.deepSleep(sleepTimeS * 1000000);
  
} // setup()


//********************* loop() *********************
void loop() {
// Uncomment for testing
/*
  static float fTemperature;
  // Process temperature
  fTemperature = readTemp();
  Serial.println(fTemperature);
  if(fTemperature < 70.0)
  {
    Serial.println("Temp low!");
  }
  delay(20);
*/ 
} // loop()
