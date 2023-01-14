// cmpe 331 project


#include <WiFi.h>
#include "SHT31.h"
#include <Firebase_ESP_Client.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

// Define the RTDB URL
#define DATABASE_URL "https://dburl-rtdb.europe-west1.firebasedatabase.app"  // firebase rtdb connection url
#define DATABASE_SECRET "xxdbsecretxx" // firebase rtdb secret
// Define the Firebase Data object
FirebaseData fbdo;

// Define the FirebaseAuth data for authentication data
FirebaseAuth auth;

// Define the FirebaseConfig data for config data
FirebaseConfig config;


const char* ssid = "cymn_hotspot";     // Wi-Fi SSID
const char* password = "cymnpass331";  // Wi-Fi Password


// Define NTP Client to get timestamp
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);


#define sht31clkpin 22
#define sht31datapin 21

SHT31 sht;

float temperature, humidity;

unsigned long sendDataPrevMillis = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  sht.begin(sht31datapin, sht31clkpin);

  //ESP32 connects to your wifi -----------------------------------
  WiFi.mode(WIFI_STA);  //Connect to your wifi
  WiFi.begin(ssid, password);

  Serial.println("Connecting to ");
  Serial.print(ssid);

  //Wait for WiFi to connect
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
  //----------------------------------------------------------------

  config.database_url = DATABASE_URL;
  Firebase.reconnectWiFi(true);
  config.signer.tokens.legacy_token = DATABASE_SECRET;

  Firebase.begin(&config, &auth);

  timeClient.begin();
  timeClient.setTimeOffset(10800);
  timeClient.update();
}

void loop() {
  // put your main code here, to run repeatedly:

  while (timeClient.getEpochTime() < 20800) {
    timeClient.update();
  }

  sht.read();  // sensor reads from environment

  temperature = sht.getTemperature();  // get temperature data from the sensor
  humidity = sht.getHumidity();        // get humidity data from the sensor
  Serial.println("Temperature: " + (String)temperature + " Celcius");
  Serial.println("Humidity: " + (String)humidity + "%");


  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    // now we will set the timestamp value at Ts
    //json.set("Ts/.sv", "timestamp"); // .sv is the required place holder for sever value which currently supports only string "timestamp" as a value
    String ts = (String)timeClient.getEpochTime();
    Serial.println(timeClient.getFormattedTime());
    Serial.println("Timestamp: " + ts);
    Serial.printf("Set temperature... %s\n", Firebase.RTDB.setFloat(&fbdo, "/test/" + ts + "/temperature", temperature) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Set humidity... %s\n", Firebase.RTDB.setFloat(&fbdo, "/test/" + ts + "/humidity", humidity) ? "ok" : fbdo.errorReason().c_str());
  }
  delay(3600000);  // wait for 1 hour to return to start of the loop
}
