#include "api_os.h"
#include "api_debug.h"
#include "api_event.h"

#include "gps.h"
#include "gps_parse.h"
#include "api_network.h"
#include "api_lbs.h"

#include "network.h"
#include "gps_agps.h"

#define GPS_AGPS_TASK_STACK_SIZE (2048 * 2)
#define GPS_AGPS_TASK_PRIORITY 0
#define GPS_AGPS_TASK_NAME "GPS_AGPS Task"

static bool flag = true;

void GPS_AGPS_Task(void *pData)
{
    int count = 0;
    GPS_Info_t *gpsInfo = Gps_GetInfo();
    uint8_t buffer[300];

    OS_WaitForSemaphore(semNetworkSuccess, OS_WAIT_FOREVER);
    OS_ReleaseSemaphore(semNetworkSuccess);

    OS_Sleep(10000);

    //open GPS hardware(UART2 open either)
    GPS_Init();
    GPS_Open(NULL);

    //wait for gps start up, or gps will not response command
    while (gpsInfo->rmc.latitude.value == 0)
        OS_Sleep(1000);

    // set gps nmea output interval
    for (uint8_t i = 0; i < 5; ++i)
    {
        bool ret = GPS_SetOutputInterval(10000);
        Trace(1, "set gps ret:%d", ret);
        if (ret)
            break;
        OS_Sleep(1000);
    }

    if (!GPS_GetVersion(buffer, 150))
        Trace(1, "get gps firmware version fail");
    else
        Trace(1, "gps firmware version:%s", buffer);

    if (!GPS_SetOutputInterval(1000))
        Trace(1, "set nmea output interval fail");

    Trace(1, "init ok");

    while (1)
    {
        if (!Network_GetCellInfoRequst())
        {
            Trace(1, "network get cell info fail");
        }
        else
        {
            flag = false;
        }
        while (!flag)
        {
            Trace(1, "wait for lbs result");
            OS_Sleep(2000);
        }
        Trace(1, "times count:%d", ++count);

        //show fix info
        uint8_t isFixed = gpsInfo->gsa[0].fix_type > gpsInfo->gsa[1].fix_type ? gpsInfo->gsa[0].fix_type : gpsInfo->gsa[1].fix_type;
        char *isFixedStr;
        if (isFixed == 2)
            isFixedStr = "2D fix";
        else if (isFixed == 3)
        {
            if (gpsInfo->gga.fix_quality == 1)
                isFixedStr = "3D fix";
            else if (gpsInfo->gga.fix_quality == 2)
                isFixedStr = "3D/DGPS fix";
        }
        else
            isFixedStr = "no fix";

        //convert unit ddmm.mmmm to degree(°)
        int temp = (int)(gpsInfo->rmc.latitude.value / gpsInfo->rmc.latitude.scale / 100);
        double latitude = temp + (double)(gpsInfo->rmc.latitude.value - temp * gpsInfo->rmc.latitude.scale * 100) / gpsInfo->rmc.latitude.scale / 60.0;
        temp = (int)(gpsInfo->rmc.longitude.value / gpsInfo->rmc.longitude.scale / 100);
        double longitude = temp + (double)(gpsInfo->rmc.longitude.value - temp * gpsInfo->rmc.longitude.scale * 100) / gpsInfo->rmc.longitude.scale / 60.0;

        snprintf(buffer, sizeof(buffer), "GPS fix mode:%d, BDS fix mode:%d, fix quality:%d, satellites tracked:%d, gps sates total:%d, is fixed:%s, coordinate:WGS84, Latitude:%f, Longitude:%f, unit:degree,altitude:%f", gpsInfo->gsa[0].fix_type, gpsInfo->gsa[1].fix_type,
                 gpsInfo->gga.fix_quality, gpsInfo->gga.satellites_tracked, gpsInfo->gsv[0].total_sats, isFixedStr, latitude, longitude, gpsInfo->gga.altitude);
        //show in tracer
        Trace(2, buffer);
        //send to UART1
        // UART_Write(UART1, buffer, strlen(buffer));
        // UART_Write(UART1, "\r\n\r\n", 4);

        OS_Sleep(10000);
    }
}

void GPS_AGPS_EventDispatch(API_Event_t *pEvent)
{
    switch (pEvent->id)
    {
    case API_EVENT_ID_NETWORK_CELL_INFO:
    {
        uint8_t number = pEvent->param1;
        Network_Location_t *location = (Network_Location_t *)pEvent->pParam1;
        Trace(2, "network cell infomation,serving cell number:1, neighbor cell number:%d", number - 1);

        float longitude, latitude;
        if (!LBS_GetLocation(location, number, 15, &longitude, &latitude))
        {
            Trace(1, "===LBS get location fail===");
        }
        else
        {
            Trace(1, "===LBS get location success,latitude:%d.%d,longitude:%d.%d===", (int)latitude, (int)((latitude - (int)latitude) * 100000),
                  (int)longitude, (int)((longitude - (int)longitude) * 100000));
        }
        flag = true;
        break;
    }
    case API_EVENT_ID_GPS_UART_RECEIVED:
        // Trace(1,"received GPS data,length:%d, data:%s,flag:%d",pEvent->param1,pEvent->pParam1,flag);
        GPS_Update(pEvent->pParam1, pEvent->param1);
        break;
    default:
        break;
    }
}

void GPS_AGPS_TaskStart(void)
{
    OS_CreateTask(GPS_AGPS_Task,
                  NULL, NULL, GPS_AGPS_TASK_STACK_SIZE, GPS_AGPS_TASK_PRIORITY, 0, 0, GPS_AGPS_TASK_NAME);
}
