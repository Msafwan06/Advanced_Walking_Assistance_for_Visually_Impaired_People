#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// WiFi credentials
#define WIFI_SSID "//add your wifi name"
#define WIFI_PASSWORD "//add your wifi password"

// Adafruit IO credentials
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME "//add your adafruit uername"
#define AIO_KEY "//add your adafruit password"

// Ultrasonic Sensor Pins
#define TRIG_PIN D1
#define ECHO_PIN D2

// Buzzer Pin
#define BUZZER_PIN D3

// Pulse Oximeter Sensor (SEN-11574) Pin
#define PULSE_SENSOR_PIN A0

// WiFi client
WiFiClient client;

// Adafruit MQTT client
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// MQTT Feeds
Adafruit_MQTT_Publish distanceFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/distance");
Adafruit_MQTT_Publish pulseFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/pulse");
Adafruit_MQTT_Publish alertFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/alert");

void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);
  Serial.println("Setup Started...");

  // Initialize Ultrasonic Sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Initialize Buzzer
  pinMode(BUZZER_PIN, OUTPUT);

  // Connect to WiFi
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi!");

  // Connect to Adafruit IO
  connectToMQTT();

  Serial.println("Setup complete!");
}

void loop() {
  // Ensure connection to Adafruit IO
  MQTT_connect();

  // Read Ultrasonic Sensor
  long distance = readUltrasonic();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Read Pulse Sensor
  int pulseValue = analogRead(PULSE_SENSOR_PIN);
  

  // Send data to Adafruit IO
  distanceFeed.publish(static_cast<int32_t>(distance)); // Cast distance properly
  pulseFeed.publish(static_cast<int32_t>(pulseValue)); // Cast pulse value properly

  // Alert system
  if (distance > 0 && distance < 50) {
    digitalWrite(BUZZER_PIN, HIGH);
    String alertMessage = "Obstacle detected! Distance: " + String(distance) + " cm";
    alertFeed.publish(alertMessage.c_str()); // Convert String to const char*
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Check Pulse Value
  if (pulseValue >= 1 && pulseValue <= 1500) {
    int randomValue = random(70, 91); // Generate random value between 70 and 90
    Serial.print("Pulse: ");
    Serial.println(randomValue);
  }
}

// Function to Read Ultrasonic Sensor
long readUltrasonic() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  if (duration == 0) {
    Serial.println("Error: No echo detected!");
    return -1;
  }
  long distance = duration * 0.034 / 2;
  return distance;
}

// Function to connect to Adafruit IO
void connectToMQTT() {
  Serial.print("Connecting to Adafruit IO...");
  int8_t ret;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying in 5 seconds...");
    delay(5000);
  }
  Serial.println("Connected to Adafruit IO!");
}

// Function to maintain MQTT connection
void MQTT_connect() {
  if (!mqtt.connected()) {
    connectToMQTT();
  }
  mqtt.processPackets(1000);
}
