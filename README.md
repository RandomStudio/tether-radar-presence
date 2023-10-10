# Tether Radar 5.8GHz

Intended for use on DFRobot SEN0521 radar module.

Publishes `{"sensorOK":false}` on startup then `{"sensorOK":true}` after first successful value reading.

Publishes presence 0/1 (on state **change** only) values on topics like `dfrobotSEN0521/94:B9:7E:6B:50:A8/presence`

## TODO
It seems that the library is capable of adjusting settings at runtime, for sensitivity, range, etc. It could be useful to subscribe to remote control messages via Tether to set these values on command.