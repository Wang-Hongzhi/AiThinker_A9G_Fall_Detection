#ifndef _MQTT_H_
#define _MQTT_H_


#define BROKER_IP  "47.95.193.110"
#define BROKER_PORT 1883
#define CLIENT_USER "xz"
#define CLIENT_PASS "123456"
#define SUBSCRIBE_TOPIC "whz/tumble_detector/app"
#define PUBLISH_TOPIC   "whz/tumble_detector/device"
#define PUBLISH_INTERVAL 10000 //10s
#define PUBLISH_PAYLOEAD "hahaha"

extern HANDLE semMqttStart;

void MQTT_TaskStart(void);

#endif

