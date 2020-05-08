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

#define MAIN_TASK_STACK_SIZE (2048 * 2)
#define MAIN_TASK_PRIORITY 0
#define MAIN_TASK_NAME "Main Task"


static HANDLE mainTaskHandle = NULL;

static void EventDispatch(API_Event_t *pEvent)
{
    switch (pEvent->id)
    {
    case API_EVENT_ID_NO_SIMCARD:
        Trace(2, "!!NO SIM CARD%d!!!!", pEvent->param1);
        break;
    case API_EVENT_ID_SIMCARD_DROP:
        Trace(2, "!!SIM CARD%d DROP!!!!", pEvent->param1);
        break;
    case API_EVENT_ID_SYSTEM_READY:
        Trace(2, "system initialize complete");
        break;
    case API_EVENT_ID_NETWORK_REGISTER_DENIED:
    case API_EVENT_ID_NETWORK_REGISTER_NO:
    case API_EVENT_ID_NETWORK_REGISTERED_HOME:
    case API_EVENT_ID_NETWORK_REGISTERED_ROAMING:
    case API_EVENT_ID_NETWORK_DETACHED:
    case API_EVENT_ID_NETWORK_ATTACH_FAILED:
    case API_EVENT_ID_NETWORK_ATTACHED:
    case API_EVENT_ID_NETWORK_DEACTIVED:
    case API_EVENT_ID_NETWORK_ACTIVATE_FAILED:
    case API_EVENT_ID_NETWORK_ACTIVATED:
    case API_EVENT_ID_SIGNAL_QUALITY:
        NetworkEventDispatch(pEvent);
        break;

    default:
        break;
    }
}

void MainTask(void *pData)
{
    API_Event_t *event = NULL;

    MQTT_TaskStart();
    GPIO_TaskStart();
    IMU_TaskStart();

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


