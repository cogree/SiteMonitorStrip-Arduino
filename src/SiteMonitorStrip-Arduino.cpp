#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include "stripcontroller.h"

#define LED_DRIVER_PIN 1
#define STATUS_LED_INDEX 0
#define NUM_LEDS 60

WiFiClient _espClient;
PubSubClient _client(_espClient);
NeoPixel_StripController* _controller;

const char* _ssid = "";
const char* _password = "";
const char* _mqtt_server = "broker.mqtt-dashboard.com";
const char* _topic = "insight/led_column";
const char* _clientId = "InsightLightboxClient-20190219";

void setup_wifi();
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);


void setup() {
  Serial.begin(115200);
  _controller = new NeoPixel_StripController(NUM_LEDS, LED_DRIVER_PIN, STATUS_LED_INDEX);
  _controller -> begin();
  _controller -> set_status_led(COLOR_RED);
  setup_wifi();
  _client.setServer(_mqtt_server, 1883);
  _client.setCallback(callback);
  _controller -> set_status_led(COLOR_YELLOW);
}

long _lastConnectedCheck;

void loop() {  
  unsigned long now_millis = millis();
  if(now_millis - _lastConnectedCheck >= 2000) {  
    _lastConnectedCheck = now_millis;
    if(!_client.connected())
      reconnect();
  }
    
  _client.loop();
  _controller -> loop();
  delay(10);
}

void setup_wifi() {

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println("Connecting to " + String(_ssid));

  delay(10);
  WiFi.begin(_ssid, _password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  delay(10);
  randomSeed(micros());

  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: " + String(WiFi.localIP()));

  _lastConnectedCheck = millis();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  String payloadStr;
  for (int i = 0; i < length; i++)
    payloadStr += (char)payload[i];
  Serial.println('"' + payloadStr + '"');

  if(payloadStr.startsWith("[") && payloadStr.endsWith("]")) {
    int r, g, b, t, bl;
    int returnVal = sscanf(payloadStr.c_str(), "[%d,%d,%d:%d:%d]", &r, &g, &b, &t, &bl);

    if(returnVal != 5) {
     Serial.println("Bad command format. Only matched " + String(returnVal) + " arguments.");
     return;
    }

    uint32_t color = 0;
    color |= (uint32_t) (r & 0xFF) << 16;
    color |= (uint32_t) (g & 0xFF) << 8;
    color |= (uint32_t) (b & 0xFF); 
    uint16_t duration = t & 0xFFFF;
    bool blink = (bl & 0x01) > 0;

    if(blink) {
      _controller -> add_led_with_blink(color, duration, 500);
    } else {
      _controller -> add_led(color, duration);
    }

    Serial.println("Adding with color=" + String(color) + "(r:" + String(r) + ",g:" + String(g) + ",b:" + String(b) + ");duration=" + String(duration) + ";blink=" + blink);
  } else {
    Serial.println("Doing nothing.");
  }
}

void reconnect() {
  if (!_client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (_client.connect(_clientId)) {
      Serial.println("connected");
      _client.subscribe(_topic);
      _controller -> set_status_led(COLOR_GREEN);
    } else {
      Serial.print("failed, rc=");
      Serial.println(_client.state());
      _controller -> set_status_led(COLOR_YELLOW);
    }
  }
}
