#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"

#include "api_os.h"
#include "api_debug.h"
#include "api_event.h"
#include "main.h"
#include "mqtt.h"
#include "gpio.h"
#include "network.h"
#include "imu.h"
#include "gps_agps.h"

#define MAIN_TASK_STACK_SIZE (2048 * 2)
#define MAIN_TASK_PRIORITY 0
#define MAIN_TASK_NAME "Main Task"


static HANDLE mainTaskHandle = NULL;

static void EventDispatch(API_Event_t *pEvent)
{
    Network_EventDispatch(pEvent);
    GPS_AGPS_EventDispatch(pEvent);
}

void MainTask(void *pData)
{
    API_Event_t *event = NULL;

    semNetworkSuccess = OS_CreateSemaphore(0);

    // MQTT_TaskStart();
    GPIO_TaskStart();
    // IMU_TaskStart();
    GPS_AGPS_TaskStart();

    while (1)
    {
        if (OS_WaitEvent(mainTaskHandle, (void **)&event, OS_TIME_OUT_WAIT_FOREVER))
        {
            EventDispatch(event);
            OS_Free(event->pParam1);
            OS_Free(event->pParam2);
            OS_Free(event);
        }
    }
}

void app_Main(void)
{
    mainTaskHandle = OS_CreateTask(MainTask,
                                   NULL, NULL, MAIN_TASK_STACK_SIZE, MAIN_TASK_PRIORITY, 
                                   0, 0, MAIN_TASK_NAME);
    OS_SetUserMainHandle(&mainTaskHandle);
}


