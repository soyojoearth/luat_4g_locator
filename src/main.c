#include "luat_network_adapter.h"
#include "common_api.h"
#include "luat_debug.h"
#include "luat_mem.h"
#include "luat_rtos.h"
#include "luat_mobile.h"

#include "libemqtt.h"
#include "luat_mqtt.h"

#include "global.h"

#include "task_lbs.h"

#include "task_mqtt.h"

#include "task_alarm.h"

#include "task_http.h"

static luat_rtos_task_handle mqtt_task_handle;
static luat_rtos_task_handle lbsLoc_request_task_handle;
static luat_rtos_task_handle alarm_task_handle;


static void luatos_mobile_event_callback(LUAT_MOBILE_EVENT_E event, uint8_t index, uint8_t status)
{
	if (LUAT_MOBILE_EVENT_NETIF == event)
	{
		if (LUAT_MOBILE_NETIF_LINK_ON == status)
		{
			LUAT_DEBUG_PRINT("luatos_mobile_event_callback  link ...");
			g_link_status = 1;
            LUAT_DEBUG_PRINT("网络注册成功\r\n");
			//luat_socket_check_ready(index, NULL);
		}
        else if(LUAT_MOBILE_NETIF_LINK_OFF == status || LUAT_MOBILE_NETIF_LINK_OOS == status)
        {
            LUAT_DEBUG_PRINT("luatos_mobile_event_callback  error ...");
			LUAT_DEBUG_PRINT("网络未注册成功\r\n");
            g_link_status = 0;
        }
	}
	else if(LUAT_MOBILE_EVENT_SIM == event){
		switch (status)
		{
		case LUAT_MOBILE_SIM_READY:
			LUAT_DEBUG_PRINT("SIM卡已插入\r\n");
			break;
		case LUAT_MOBILE_NO_SIM:
		default:
			break;
		}
	}
	else if(LUAT_MOBILE_EVENT_CELL_INFO == event){
		switch (status)
		{
		case LUAT_MOBILE_CELL_INFO_UPDATE:
			LUAT_DEBUG_PRINT("周期性搜索小区信息完成一次\r\n");
			break;
		default:
			break;
		}
	}
}



static void luat_libemqtt_init(void)
{
	luat_mobile_set_sim_detect_sim0_first();
	luat_mobile_event_register_handler(luatos_mobile_event_callback);

	// MQTT任务
	luat_rtos_task_create(&mqtt_task_handle, 2 * 1024, 10, "libemqtt", luat_mqtt_task, NULL, 16);

	//LBS定位
    luat_rtos_task_create(&lbsLoc_request_task_handle, 4 * 2048, 50, "lbs_task", lbsloc_request_task, NULL, NULL);

	//告警检测任务
    luat_rtos_task_create(&alarm_task_handle, 4 * 1024, 50, "alarm_task", luat_alarm_task, NULL, NULL);

	//每次上电，HTTP获取MQTT账号
    luat_rtos_task_create(&http_task_handle, 4 * 1024, 20, "http_task", luat_http_task, NULL, 16);

}

INIT_TASK_EXPORT(luat_libemqtt_init, "1");
