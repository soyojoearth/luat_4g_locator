#include <stdio.h>
#include <math.h>
#include "luat_network_adapter.h"
#include "common_api.h"
#include "luat_debug.h"
#include "luat_mem.h"
#include "luat_rtos.h"
#include "luat_mobile.h"

#include "global.h"

#include "task_alarm.h"

#include "task_lbs.h"

#define PI                      3.1415926
#define EARTH_RADIUS            6378.137        //地球近似半径

bool isOutGeofencing = false;//默认当前没有越界

double radian(double d);
double get_distance(double lat1, double lng1, double lat2, double lng2);

// 求弧度
double radian(double d)
{
    return d * PI / 180.0;   //角度1˚ = π / 180
}

//计算距离
double get_distance(double lat1, double lng1, double lat2, double lng2)
{
    double radLat1 = radian(lat1);
    double radLat2 = radian(lat2);
    double a = radLat1 - radLat2;
    double b = radian(lng1) - radian(lng2);
    
    double dst = 2 * asin((sqrt(pow(sin(a / 2), 2) + cos(radLat1) * cos(radLat2) * pow(sin(b / 2), 2) )));
    
    dst = dst * EARTH_RADIUS;
    dst= round(dst * 10000) / 10000;
    return dst;
}


uint8_t checkIsOutGeofencing(){

    //检查电子围栏，是否越界
    // uint8_t dpValue_data_geofencing[252] = {0};//12个字节1个围栏，一共可以存21个围栏
    // int16_t dpValue_data_geofencing_length = 0;

    int32_t distanceLimit = 0;
    double distanceLimitDouble = 0;

    int32_t latInt = 2410000000;
    int32_t lonInt = 2410000000;

    double latDouble = 2410000000;
    double lonDouble = 2410000000;

    
    // LUAT_DEBUG_PRINT("Location lbs_lat: %0.7f lbs_lon: %0.7f \n", lbs_lat, lbs_lon);


    if (dpValue_data_geofencing_length == 0)
    {
        // LUAT_DEBUG_PRINT("Geofencing empty");
        return 0;//没有越界
    }
    // else{
    //     // LUAT_DEBUG_PRINT("Geofencing :%i", dpValue_data_geofencing_length);
    // }
    

    int k = 0;
    for (size_t i = 0; i < dpValue_data_geofencing_length; i+=12)
    {

        latInt = ((dpValue_data_geofencing[i+0] & 0xffffffff) << 24) | 
					((dpValue_data_geofencing[i+1] & 0xffffffff) << 16) | 
					((dpValue_data_geofencing[i+2] & 0xffffffff) << 8) | 
					((dpValue_data_geofencing[i+3] & 0xffffffff) << 0);

                    latDouble = (double)latInt / (double)10000000;

        lonInt = ((dpValue_data_geofencing[i+4] & 0xffffffff) << 24) | 
					((dpValue_data_geofencing[i+5] & 0xffffffff) << 16) | 
					((dpValue_data_geofencing[i+6] & 0xffffffff) << 8) | 
					((dpValue_data_geofencing[i+7] & 0xffffffff) << 0);

                    lonDouble = (double)lonInt / (double)10000000;


        distanceLimit = ((dpValue_data_geofencing[i+8] & 0xffffffff) << 24) | 
					((dpValue_data_geofencing[i+9] & 0xffffffff) << 16) | 
					((dpValue_data_geofencing[i+10] & 0xffffffff) << 8) | 
					((dpValue_data_geofencing[i+11] & 0xffffffff) << 0);

        distanceLimitDouble = (double)distanceLimit / (double)100000;

        // LUAT_DEBUG_PRINT("Geofencing %i, latDouble = %0.7f lonDouble = %0.7f\n distanceLimit = %0.7f", k, latDouble, lonDouble, distanceLimitDouble);

        k++;

        // lbs_lat;//纬度
        // lbs_lon;//经度
        
        double dst = get_distance(lbs_lat, lbs_lon, latDouble, lonDouble);

        // LUAT_DEBUG_PRINT("dst = %0.3fkm\n", dst);  //dst = 9.281km

        if (dst < distanceLimitDouble || distanceLimitDouble == 0)
        {
            //只要在其中任何一个围栏之内，就没有越界
            return 0;
        }
                
    }

    return 1;//不在任何一个围栏内，越界了
    
}


void checkAll(){

    int8_t res = 0;

    //检查是否围栏越界
    res = checkIsOutGeofencing();

    if (res)
    {
        // LUAT_DEBUG_PRINT("checkIsOutGeofencing:YES\n");
        if (!isOutGeofencing)//是不是刚越界
        {
            //越界了
            LUAT_DEBUG_PRINT("OutGeofencing First\n");
            isOutGeofencing = true;
            if(dpValue_alarmFlag & (1 << 3)){//通知开关是否打开
                LUAT_DEBUG_PRINT("Alarm 3\n");
                dpValue_alarmStatus |= (1 << 3);//出围栏置1
            }
            if(dpValue_alarmFlag & (1 << 2)){//通知开关是否打开
                LUAT_DEBUG_PRINT("Alarm 2\n");
                dpValue_alarmStatus &= ~(1 << 2);//进围栏置0
            }
        }
    }
    else{
        // LUAT_DEBUG_PRINT("checkIsOutGeofencing:NO\n");
        if (isOutGeofencing)//是不是刚回来
        {
            //回界了
            LUAT_DEBUG_PRINT("BackGeofencing First\n");
            isOutGeofencing = false;
            if(dpValue_alarmFlag & (1 << 3)){//通知开关是否打开
                LUAT_DEBUG_PRINT("Alarm 3\n");
                dpValue_alarmStatus &= ~(1 << 3);//出围栏置0
            }
            if(dpValue_alarmFlag & (1 << 2)){//通知开关是否打开
                LUAT_DEBUG_PRINT("Alarm 2\n");
                dpValue_alarmStatus |= (1 << 2);//进围栏置1
            }
        }
    }

    //检查温湿度

    //检查其它。。。
    
}

void luat_alarm_task(void *param)
{
    while(1){

		//检测任务
        checkAll();

        //其它。。。


		luat_rtos_task_sleep(1000);

	}
}