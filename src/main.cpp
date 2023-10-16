#include <ArduinoJson.h>
#include <ArduinoMqttClient.h>
#include <WiFi.h>

#include "DFRobot_Microwave_Radar_Module.h"
#include "settings.h"

#if (defined(ARDUINO_AVR_UNO) || defined(ESP8266))  // Use soft serial port
SoftwareSerial softSerial(/*rx =*/4, /*tx =*/5);
DFRobot_Microwave_Radar_Module Sensor(/*softSerial =*/&softSerial);
#elif defined(ESP32)  // use hard serial port of the pin with remapping function : Serial1
// connect module RX to MCU TX (27) and module TX to MCU RX (33)
DFRobot_Microwave_Radar_Module Sensor(/*hardSerial =*/&Serial1, /*rx =*/33, /*tx =*/27);
#else                 // use hard serial port : Serial1
DFRobot_Microwave_Radar_Module Sensor(/*harxdSerial =*/&Serial1);
#endif

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
#if USE_SERIAL == 0
    Serial.println("START (wait for config done; this can take up to a minute!)");
#endif

    pinMode(LED_PIN, OUTPUT);

#if USE_TETHER == 1
    connectWiFi();

    String id = WiFi.macAddress();
    updateOutputTopics(id);

    delay(1000);
    MQTT_connect();
#endif

    /**
       @brief Restore factory settings
    */
    Sensor.factoryReset();

    /**
       @brief Configure detection distance, 0~11m, the default is 6m
    */
    Sensor.detRangeCfg(SENSOR_RANGE);

    /**
       @brief Configure detection sensitivity, 0~9, the larger the value, the higher the sensitivity, default to be 7
    */
    Sensor.setSensitivity(SENSOR_SENSITIVITY);

    /**
       @brief Configure output delay time
    */
    Sensor.outputLatency(SENSOR_DELAY_ON, SENSOR_DELAY_OFF);

    /**
       @brief Configure output control signal interface polarity
    */
    Sensor.setGpioMode(1);
#if USE_SERIAL == 0  // only print debugging messages if we are NOT using Serial for output
    Serial.println("...Config done");
#endif
}

void loop() {
    int val = Sensor.readPresenceDetection();

    if (!sensorOK) {
        sensorOK = true;
#if USE_TETHER == 1
        sendStatus(true);
#elif USE_SERIAL == 1
        Serial.println(val);
#endif
    }

    if (val != lastPresence) {
        lastPresence = val;
#if USE_TETHER == 1
        sendPresence(val);
#elif USE_SERIAL == 1
        Serial.println(val);
#endif
    }
    // int val = 0;
    digitalWrite(LED_PIN, val);
    // Serial.println(val);
}