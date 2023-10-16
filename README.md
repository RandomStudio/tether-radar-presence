# Tether Radar 5.8GHz

Intended for use on DFRobot SEN0521 radar module. This device is suitable for (human) **presence detection** only. There is no direct access to range or velocity/movement data from this device, even if that is used internally.

## Configuration
For now, duplicate the `settings.template.h` file as `settings.h` (the latter is required to build, but ignored by Git). Adjust defined values as needed.

In future, we may use Tether (and Serial?) messages sent to the device to request runtime configuration changes.

## Usage with Tether
Ensure `#define USE_TETHER 1` is set in `settings.h` and check other Wifi/MQTT related settings there, too.

Publishes `{"sensorOK":false}` on startup then `{"sensorOK":true}` after first successful value reading.

Publishes presence 0/1 (on state **change** only) values on topics like `dfrobotSEN0521/94:B9:7E:6B:50:A8/presence`

## Usage with Serial
Ensure that `#define USE_SERIAL 1` is set in `settings.h`.

Writes `1\n` when presence (positive) is confirmed, `0\n` when no-presence (negative). 

Note that these values are only emitted when there is a **change** - not continuously.
