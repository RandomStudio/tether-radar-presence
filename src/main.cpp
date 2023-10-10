#include <ArduinoJson.h>
#include <ArduinoMqttClient.h>
#include <WiFi.h>

#include "DFRobot_Microwave_Radar_Module.h"

#if (defined(ARDUINO_AVR_UNO) || defined(ESP8266))  // Use soft serial port
SoftwareSerial softSerial(/*rx =*/4, /*tx =*/5);
DFRobot_Microwave_Radar_Module Sensor(/*softSerial =*/&softSerial);
#elif defined(ESP32)  // use hard serial port of the pin with remapping function : Serial1
// connect module RX to MCU TX (27) and module TX to MCU RX (33)
DFRobot_Microwave_Radar_Module Sensor(/*hardSerial =*/&Serial1, /*rx =*/33, /*tx =*/27);
#else                 // use hard serial port : Serial1
DFRobot_Microwave_Radar_Module Sensor(/*harxdSerial =*/&Serial1);
#endif

// #define WLAN_SSID "tether_2.4"
// #define WLAN_PASS "sp_ceB0ss!"
#define WLAN_SSID "Random Guest"
#define WLAN_PASS "beourguest"

#define MQTT_IP "10.112.20.165"
#define MQTT_PORT 1883
#define MQTT_USER "tether"
#define MQTT_PASS "sp_ceB0ss!"

#define AGENT_ROLE "dfrobotSEN0521"
#define LED_PIN 13

WiFiClient wifiClient;
MqttClient mqtt(wifiClient);

String agentRole = String(AGENT_ROLE);
String outputTopicStatus = agentRole + "/any/status";
String outputTopicPresence = agentRole + "/any/presence";

StaticJsonDocument<128> outputDoc;
std::string outputMessage;

bool sensorOK;
int lastPresence;

void connectWiFi() {
    Serial.println("Connecting to WiFi");
    WiFi.begin(WLAN_SSID, WLAN_PASS);
    delay(2000);
    bool ledOn = false;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("...WiFi connected");
}

void sendStatus(bool sensorOK = true) {
    outputDoc["sensorOK"] = sensorOK;

    outputMessage = "";  // clear the output string

    serializeMsgPack(outputDoc, outputMessage);  // serialize the data

    // send: retain = false, QOS 2
    mqtt.beginMessage(outputTopicStatus, false, 2);

    for (int i = 0; i < outputMessage.length(); i++) {
        mqtt.print(outputMessage[i]);
    }

    mqtt.endMessage();
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care of connecting.
void MQTT_connect() {
    // Stop if already connected.
    if (mqtt.connected()) {
        return;
    }

    Serial.print("Connecting to MQTT Broker @ ");
    Serial.print(MQTT_IP);
    Serial.println(" ... ");

    mqtt.setUsernamePassword(MQTT_USER, MQTT_PASS);

    while (!mqtt.connect(MQTT_IP, MQTT_PORT)) {
        Serial.println(mqtt.connectError());
        Serial.println("Retrying MQTT connection in 5 seconds...");
        delay(5000);  // wait 5 seconds
    }

    Serial.println("...MQTT Connected!");

    sendStatus(false);
}

void updateOutputTopics(String id) {
    outputTopicStatus = agentRole + "/" + id + "/status";
    outputTopicPresence = agentRole + "/" + id + "/presence";
}

void sendPresence(bool presence) {
    outputDoc = JsonInteger(presence);

    outputMessage = "";                          // clear the output string
    serializeMsgPack(outputDoc, outputMessage);  // serialize the data

    // send: retain = false, QOS 2
    mqtt.beginMessage(outputTopicPresence, false, 2);

    for (int i = 0; i < outputMessage.length(); i++) {
        mqtt.print(outputMessage[i]);
    }

    mqtt.endMessage();
}

void setup() {
    sensorOK = false;
    lastPresence = -1;
    Serial.begin(115200);

    // Initialize sensor
    while (!(Sensor.begin())) {
        Serial.println("Communication with device failed, please check connection");
        delay(3000);
    }
    Serial.println("START (wait for config done; this can take up to a minute!)");

    pinMode(LED_PIN, OUTPUT);

    connectWiFi();

    String id = WiFi.macAddress();
    updateOutputTopics(id);

    delay(1000);
    MQTT_connect();

    /**
       @brief Restore factory settings
    */
    Sensor.factoryReset();

    /**
       @brief Configure detection distance, 0~11m, the default is 6m
    */
    Sensor.detRangeCfg(3);

    /**
       @brief Configure detection sensitivity, 0~9, the larger the value, the higher the sensitivity, default to be 7
    */
    Sensor.setSensitivity(3);

    /**
       @brief Configure output delay time
    */
    Sensor.outputLatency(0, 1);

    /**
       @brief Configure output control signal interface polarity
    */
    Sensor.setGpioMode(1);

    Serial.println("...Config done");
}

void loop() {
    int val = Sensor.readPresenceDetection();
    if (val != lastPresence) {
        lastPresence = val;
        sendPresence(val);
    }
    // int val = 0;
    digitalWrite(LED_PIN, val);
    Serial.println(val);
}