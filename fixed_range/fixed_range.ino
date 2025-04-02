#include <SPI.h>
#include <MFRC522.h>
#include <vector>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

#define SS_PIN 21  // Adjusted for 30-pin ESP32 Dev Kit
#define RST_PIN 22  // Adjusted for 30-pin ESP32 Dev Kit

MFRC522 rfid(SS_PIN, RST_PIN);

int led_1 = 0;
int led_2 = 0;
int led_3 = 0;
int range_1 = 0;
int range_2 = 0;
int range_3 = 0;
int ledPins[] = {25, 26, 27};  // Adjusted LED pins
int potPins[] = {34, 35, 32};  // Adjusted potentiometer pins
bool game_start = false;
bool pot_1 = false;
bool pot_2 = false;
bool pot_3 = false;
int assigned = 0;
std::vector<String> game_pause = {""};
String approvedTags[6] = {"color_1", "color_2", "color_3", "color_4", "color_5", "color_6"};

String color = {};
// WiFi
const char *ssid = "BYUI_Visitor"; // Enter your Wi-Fi name
const char *password = "";  // Enter Wi-Fi password

// MQTT Broker
const char *mqtt_broker = "gigachad.website";
const char *topic = "range";
const char *topic1 = "range_done";
const char *mqtt_username = "amogus";
const char *mqtt_password = "sussyimposter";
const int mqtt_port = 8883;
char *finishedTags = "";

WiFiClientSecure espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  SPI.begin(18, 19, 23, 21);  // SCK=18, MISO=19, MOSI=23, SS=21
  rfid.PCD_Init();

  Serial.println("Tap RFID/NFC Tag on reader");

  for (int i = 0; i < 3; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  for (int i = 0; i < 3; i++) {
    pinMode(potPins[i], INPUT);
  }
  espClient.setInsecure();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the Wi-Fi network");
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public EMQX MQTT broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  client.publish(topic, "Hi, I'm ESP32 ^^");
  client.subscribe(topic);
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  if (length == 1) {
    approvedTags[assigned] = (char) payload[0];
    Serial.println(approvedTags[assigned]);
    assigned += 1;
  } else {
    game_pause[0] = (char) payload[0];
    if (game_pause[0] == "m") {
      Serial.println("meeting");
      color = "";
      for (int pin : ledPins) digitalWrite(pin, LOW);
      game_start = false;
    }
    if (game_pause[0] == "g") {
      game_start = false;
      color = "";
      for (int pin : ledPins) digitalWrite(pin, LOW);
      for (String &tag : approvedTags) tag = "";
    }
  }
}

void loop() {
  client.loop();
  if (!game_start) checkRFID();

  if (game_start) {
    led_1 = analogRead(34) * (5.0 / 1023.0);
    led_2 = analogRead(35) * (5.0 / 1023.0);
    led_3 = analogRead(32) * (5.0 / 1023.0);

    pot_1 = (led_1 < range_1 && led_1 > (range_1 - 3));
    pot_2 = (led_2 < range_2 && led_2 > (range_2 - 3));
    pot_3 = (led_3 < range_3 && led_3 > (range_3 - 3));

    digitalWrite(25, pot_1 ? HIGH : LOW);
    digitalWrite(26, pot_2 ? HIGH : LOW);
    digitalWrite(27, pot_3 ? HIGH : LOW);

    if (pot_1 && pot_2 && pot_3) {
      delay(3000);
      game_start = false;
      for (int pin : ledPins) digitalWrite(pin, LOW);
      client.publish(topic1, color.c_str());
      color = "";
    }
  }
}

void checkRFID() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    byte buffer[18];
    byte size = sizeof(buffer);
    if (rfid.MIFARE_Read(4, buffer, &size) == MFRC522::STATUS_OK) {
      String tagData = "";
      for (byte i = 0; i < size; i++) {
        if (isPrintable(buffer[i])) tagData += (char)buffer[i];
      }
      tagData.trim();
      color = tagData[3];
      for (const String &tag : approvedTags) {
        if (color == tag) {
          game_start = true;
          range_1 = random(3, 15);
          range_2 = random(3, 15);
          range_3 = random(3, 15);
          Serial.println("Authorized tag detected. Game started.");
          return;
        }
      }
      Serial.println("Unauthorized tag.");
    }
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
}
