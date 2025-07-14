#include <ESP8266WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <SoftwareSerial.h>

// Replace with your network and Adafruit IO credentials
#define WIFI_SSID "Your_WiFi_SSID"
#define WIFI_PASSWORD "Your_WiFi_Password"
#define AIO_USERNAME "Your_Adafruit_IO_Username"
#define AIO_KEY "Your_Adafruit_IO_Key"

// Adafruit IO MQTT Server
#define MQTT_SERVER "io.adafruit.com"
#define MQTT_SERVERPORT 1883

// Ultrasonic Sensor Pins
#define TRIG_PIN D1
#define ECHO_PIN D2

// Buzzer Pin
#define BUZZER_PIN D3

// Heart Rate Sensor Pin
#define HEART_RATE_PIN A0

// GPS Module Pins
#define RX_PIN D6
#define TX_PIN D5

// Initialize WiFi and MQTT
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_SERVERPORT, AIO_USERNAME, AIO_KEY);

// MQTT Feeds
Adafruit_MQTT_Publish ultrasonic_feed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/ultrasonic");
Adafruit_MQTT_Publish heart_rate_feed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/heart-rate");
Adafruit_MQTT_Publish gps_feed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/gps");

// Initialize Software Serial for GPS
SoftwareSerial gpsSerial(RX_PIN, TX_PIN);

void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);

  // Initialize GPS Serial
  gpsSerial.begin(9600);

  // Initialize Pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize WiFi
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");
}

void loop() {
  // Ensure MQTT Connection
  MQTT_connect();

  // Read Ultrasonic Sensor
  long distance = readUltrasonic();
  ultrasonic_feed.publish(distance);

  // Trigger Buzzer for Obstacles
  if (distance < 50) {
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Read Heart Rate Sensor
  int heartRate = analogRead(HEART_RATE_PIN);
  heart_rate_feed.publish(heartRate);

  // Read GPS Data
  String gpsData = readGPS();
  gps_feed.publish(gpsData.c_str());

  delay(2000); // Delay for 2 seconds
}

// Function to Read Ultrasonic Sensor
long readUltrasonic() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  long distance = duration * 0.034 / 2; // Convert to cm
  return distance;
}

// Function to Read GPS Data
String readGPS() {
  String gpsData = "";
  while (gpsSerial.available()) {
    gpsData += char(gpsSerial.read());
  }
  if (gpsData.length() == 0) {
    gpsData = "No GPS Data";
  }
  return gpsData;
}

// Function to Ensure MQTT Connection
void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()) {
    return;
  }
  Serial.print("Connecting to MQTT...");
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
  }
  Serial.println("MQTT Connected!");
}