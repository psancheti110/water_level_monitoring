/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-hc-sr04-ultrasonic-arduino/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#include "DHT.h"
#include "analogWrite.h"
#include "HTTPClient.h"
#include <Math.h> 
#include <WiFiClient.h> 
#include <WebServer.h>
#include <JSONVar.h>
#include <JSON.h>
#include <Arduino_JSON.h>
#include <time.h>
#include <WiFi.h>
#include <DHT.h>
#include <SoftwareSerial.h>

// variables
//define sound speed in cm/uS
#define SOUND_SPEED 0.0343
#define CM_TO_INCH 0.393701
#define DHTPIN 4
#define DHTTYPE DHT22 
const int trigPin = 5;
const int echoPin = 18;
const int RELAY_PIN = 26; // ESP32 pin GIOP27, which connects to the IN pin of relay
const int TANK_HEIGHT = 45;
long duration;
float distanceCm;
float distanceInch;

//wifi credentials
//char* wifi_ssid = "Chawla";
//char* wifi_pwd = "abhishek";

const char* ssid = "Bittu";
const char* password = "123456788";
//String get_target_url = "http://esw-onem2m.iiit.ac.in:443/~/in-cse/in-name/Team-12/Node-1/Data";
String post_sensordata_url = "http://127.0.1.1:8080/~/in-cse/in-name/Team-12/Data";

uint wifi_delay = 5000;
uint lastTime = 0;
HTTPClient http;


DHT dht(DHTPIN, DHTTYPE);

void setup() {
  
  pinMode(RELAY_PIN, OUTPUT);
  Serial.begin(115200); // Starts the serial communication
  
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  
  Serial.println(F("DHT22 test!"));
  dht.begin();

  // WIFI Setup
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED)
    delay(500);
  Serial.print("\nConnected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

}

void sendSensorData(String sensor_data)
{
  http.begin(post_sensordata_url.c_str());
  Serial.println(millis());
 
  // Your Domain name with URL path or IP address with path
  
  Serial.println(millis());

  
  //http.addHeader("X-M2M-Origin", "DLVosnfNhb:8FHrMl@3TX");  //CREDENTIALS
  http.addHeader("X-M2M-Origin", "admin:admin");  //CREDENTIALS
  http.addHeader("Content-Type", "application/json;ty=4");

  String ciRepresentation =
    "{\"m2m:cin\": {"
    "\"con\":\"" + sensor_data + "\""
    "}}";

  Serial.println(ciRepresentation);
  // Send HTTP GET request
  int httpResponseCode = http.POST(ciRepresentation);
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  Serial.println(millis());
    
  // Free resources
  Serial.println(millis());
  http.end();
}

String encrypt(int n)
{
    // char array to store hexadecimal number
    char hexaDeciNum[100];
 
    // counter for hexadecimal number array
    int i = 0;
    while (n != 0) {
        // temporary variable to store remainder
        int temp = 0;
 
        // storing remainder in temp variable.
        temp = n % 16;
 
        // check if temp < 10
        if (temp < 10) {
            hexaDeciNum[i] = temp + 48;
            i++;
        }
        else {
            hexaDeciNum[i] = temp + 55;
            i++;
        }
 
        n = n / 16;
    }
    String res="";
    // printing hexadecimal number array in reverse order
    for (int j = i - 1; j >= 0; j--)
        res+=hexaDeciNum[j];
    
    return res;
}

// Use folding on a string, summed 4 bytes at a time
int sfold_hash(String s, int M) {
  long sum = 0, mul = 1;
  for (int i = 0; i < s.length(); i++) {
    mul = (i % 4 == 0) ? 1 : mul * 256;
    sum += s.charAt(i) * mul;
  }
  if(sum < 0)
    return (int)((-1*sum)%M);
  else
    return (int)(sum%M);
}

void loop() {
    
  //Speed of sound m/s = 331.4 + (0.606 * Temp) + (0.0124 * Humidity)
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false); 
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  Serial.print("Temperature: ");
  Serial.println(t);
  Serial.print("Humidity: ");
  Serial.println(h);
 
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  double speedofsound = (331.4 +(0.606 * t) + (0.0124 * h))/(double)10000;
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  Serial.print("Speed of Sound: ");
  Serial.print(SOUND_SPEED, 5);
  Serial.println(" cm/uS");
  
  // Convert to inches
  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  Serial.print("Distance (inch): ");
  Serial.println(distanceInch);

  int pump_status=2;
  if(distanceCm <= 10)
  {
    //Turn on
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("Turning Pump On");
    pump_status = 1; 
  }
  else if(distanceCm >= 50)
  {
    //Turnoff  
    digitalWrite(RELAY_PIN, HIGH);
    Serial.println("Turning pump off");
    pump_status = 2;
  }

  int water_level = distanceCm; 
  String water_level_encrypted = encrypt(water_level);
  String pump_status_encrypted = encrypt(pump_status);
    
  String sensor_data = water_level_encrypted + ";" + pump_status_encrypted;
  String to_be_hashed = water_level_encrypted + pump_status_encrypted;
  int hash = sfold_hash(to_be_hashed, 50);
  Serial.print("Hash : ");
  Serial.println(hash);
  Serial.print("SensorData Encrypted : ");
  Serial.println(sensor_data);
  sensor_data+=";" + String(hash);
  sendSensorData(sensor_data);
  
  delay(20000);
}
