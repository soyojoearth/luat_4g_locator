#ifndef _TASK_MQTT_H
#define _TASK_MQTT_H

#include "commontypedef.h"

extern char* mqttUser;
extern char* mqttPwd;
extern char* mqttDeviceId;

extern char* mqtt_sub_topic;
extern char* mqtt_pub_topic;

void luat_mqtt_task(void *param);

#endif