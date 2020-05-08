
#include "api_os.h"
#include "api_debug.h"
#include "api_event.h"
#include "imu.h"
#include "icm20602.h"


#define IMU_TASK_STACK_SIZE (1024 * 2)
#define IMU_TASK_PRIORITY 1
#define IMU_TASK_NAME "IMU Task"

static HANDLE imuTaskHandle = NULL;

void IMU_Task(void *pData)
{
    short imuData[6];
    ICM20602_Init();
    Trace(1, "ICM Init Success");
    while(1)
    {
        ICM_Get_Raw_data(&imuData[0], &imuData[1], &imuData[2], &imuData[3], &imuData[4], &imuData[5]);
        Trace(1, "ax:%d, ay:%d, az:%d, gx:%d, gy:%d, gz:%d", imuData[0], imuData[1], imuData[2], imuData[3], imuData[4], imuData[5]);
        OS_Sleep(1000);
    }
}

void IMU_TaskStart(void)
{
    imuTaskHandle = OS_CreateTask(IMU_Task, NULL, NULL, IMU_TASK_STACK_SIZE, IMU_TASK_PRIORITY, 0, 0, IMU_TASK_NAME);
}


