#include "api_os.h"
#include "api_debug.h"
#include "api_event.h"
#include "api_info.h"
#include "api_hal_gpio.h"
#include "api_hal_pm.h"
#include "gpio.h"

#define GPIO_TASK_STACK_SIZE (1024 * 2)
#define GPIO_TASK_PRIORITY 1
#define GPIO_TASK_NAME "GPIO Task"

static HANDLE gpioTaskHandle = NULL;

void GPIO_Task(void *pData)
{
    static GPIO_LEVEL ledBlueLevel = GPIO_LEVEL_LOW;

    GPIO_config_t gpioLedBlue = {
        .mode = GPIO_MODE_OUTPUT,
        .pin = GPIO_PIN25,
        .defaultLevel = GPIO_LEVEL_LOW};

    PM_PowerEnable(POWER_TYPE_VPAD, true);

    gpioLedBlue.pin = GPIO_PIN27;
    GPIO_Init(gpioLedBlue);

    while (1)
    {
        ledBlueLevel = (ledBlueLevel == GPIO_LEVEL_HIGH) ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH;
        for (int i = 0; i < GPIO_PIN_MAX; ++i)
        {
            gpioLedBlue.pin = i;
            GPIO_SetLevel(gpioLedBlue, ledBlueLevel); //Set level
        }
        OS_Sleep(500); //Sleep 500 ms
    }
}

void GPIO_TaskStart(void)
{
    gpioTaskHandle = OS_CreateTask(GPIO_Task, NULL, NULL, GPIO_TASK_STACK_SIZE, GPIO_TASK_PRIORITY, 0, 0, GPIO_TASK_NAME);
}