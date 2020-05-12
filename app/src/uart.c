#include "api_os.h"
#include "api_debug.h"

#include "api_hal_uart.h"

#include "uart.h"


static void OnUart1ReceivedData(UART_Callback_Param_t param)
{
    UART_Write(UART1, param.buf, param.length);
    Trace(1, "uart1 interrupt received data,length:%d:data:%s", param.length, param.buf);
}

void usrUART_Init(void)
{
    UART_Config_t config = {
        .baudRate = UART_BAUD_RATE_9600,
        .dataBits = UART_DATA_BITS_8,
        .stopBits = UART_STOP_BITS_1,
        .parity = UART_PARITY_NONE,
        .rxCallback = OnUart1ReceivedData,
        .useEvent = false,
    };

    UART_Init(UART1, config);
}

