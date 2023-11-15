#define USE_TETHER 0
#define USE_SERIAL 1

// Detection distance in metres (integer), 0~11m; default 6m
#define SENSOR_RANGE 3

// Sensitivity value (integer), 0-9; default is 7
// This is NOT related to SENSOR_RANGE above. Higher values will
// make small movements within the range more likely to trigger
// presence = true.
#define SENSOR_SENSITIVITY 5

// Time in seconds (integer) to delay confirming ON (positive, presence)
// and OFF (negative, no presence). 0,1 provides fast-as-possible "positive"
// and slightly delayed "negative" signals.
#define SENSOR_DELAY_ON 0
#define SENSOR_DELAY_OFF 1

#define WLAN_SSID "Random Guest"
#define WLAN_PASS "beourguest"

#define MQTT_IP "10.112.20.165"
#define MQTT_PORT 1883
#define MQTT_USER "tether"
#define MQTT_PASS "sp_ceB0ss!"

#define AGENT_ROLE "dfrobotSEN0521"
