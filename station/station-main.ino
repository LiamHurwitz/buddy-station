// THIS CODE WILL NEED TO BE CHANGED DRASTICALLY TO BROADCAST WEATHER DATA DIRECTLY TO A COMPUTER OR AT LEAST FIND AN ALTERNATIVE TO THINGSPEAK. IT'S PAID

#include <WiFi.h>
#include <WiFiMulti.h>

WiFiMulti WiFiMulti;

const char* ssid     = "YOUR_SSID"; // Your SSID (Name of your WiFi)
const char* password = "YOUR_WIFI_PASSWORD"; //Your Wifi password

const char* host = "api.thingspeak.com";
String api_key = "YOUR_API_KEY"; // Your API Key provied by thingspeak

//Library for BME280 Sensor
#include <DFRobot_BME280.h>

#define SEA_LEVEL_PRESSURE  1013.25f

DFRobot_BME280 bme; //I2C

float temp, pressure, hum, alt; //Variables for the BME280 Sensor

//Library for ESP Sleep
#include "esp_deep_sleep.h"

#define uS_TO_m_FACTOR 60000000   // Conversion factor for micro seconds to minutes
#define TIME_TO_SLEEP  1        // Time ESP32 will go to sleep (in minutes)


void setup() {

  Serial.begin(115200);
  delay(10);

  Connect_to_Wifi();

  // I2c default address is 0x77, if the need to change please modify bme.begin(Addr)
  if (!bme.begin()) {
    Serial.println("No sensor device found, check line or address!");
  }

  delay(100);

  Get_Values();

  Send_Data();


  esp_deep_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_m_FACTOR);

  Serial.println("Going to sleep now");

  esp_deep_sleep_start();


}

void loop() {
  // put your main code here, to run repeatedly:

}


void Get_Values()
{

  temp = bme.temperatureValue();
  delay(100);

  pressure = bme.pressureValue() / 100.0F;
  hum = bme.humidityValue();
  alt = bme.altitudeValue(SEA_LEVEL_PRESSURE);

  delay(100);

  Serial.println("Collect data");

  Serial.print("Temperature :");
  Serial.print(temp);
  Serial.println(" C");

  Serial.print("Pressure:");
  Serial.print(pressure);
  Serial.println(" hPa");

  Serial.print("Humidity :");
  Serial.print(hum);
  Serial.println(" %");

  Serial.print("Approx. Altitude:");
  Serial.print(alt);
  Serial.println(" m");

  Serial.println("------END------");

  //Place BME280 into sleep mode
  BME280_Sleep();

}


void Connect_to_Wifi()
{

  // We start by connecting to a WiFi network
  WiFiMulti.addAP(ssid, password);

  Serial.println();
  Serial.println();
  Serial.print("Wait for WiFi... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void Send_Data()
{

  Serial.println("Prepare to send data");

  // Use WiFiClient class to create TCP connections
  WiFiClient client;

  const int httpPort = 80;

  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  else
  {
    String data_to_send = api_key;
    data_to_send += "&field1=";
    data_to_send += String(temp);
    data_to_send += "&field2=";
    data_to_send += String(hum);
    data_to_send += "&field3=";
    data_to_send += String(pressure);
    data_to_send += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + api_key + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(data_to_send.length());
    client.print("\n\n");
    client.print(data_to_send);

    delay(1000);
  }

  client.stop();



}

void BME280_Sleep()
{

  //Serial.println("BME280 to Sleep mode");

  Wire.beginTransmission(0x77);
  Wire.write((uint8_t)BME280_REGISTER_CONTROL);
  Wire.write((uint8_t)0b00);
  Wire.endTransmission();
}
