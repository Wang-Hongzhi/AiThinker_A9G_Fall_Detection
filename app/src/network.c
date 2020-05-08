#include "api_os.h"
#include "api_debug.h"
#include "api_event.h"
#include "api_network.h"
#include "api_socket.h"
#include "api_info.h"
#include "mqtt.h"
#include "network.h"


bool AttachActivate()
{
    uint8_t status;
    bool ret = Network_GetAttachStatus(&status);
    if (!ret)
    {
        Trace(2, "get attach staus fail");
        return false;
    }
    Trace(2, "attach status:%d", status);
    if (!status)
    {
        ret = Network_StartAttach();
        if (!ret)
        {
            Trace(2, "network attach fail");
            return false;
        }
    }
    else
    {
        ret = Network_GetActiveStatus(&status);
        if (!ret)
        {
            Trace(2, "get activate staus fail");
            return false;
        }
        Trace(2, "activate status:%d", status);
        if (!status)
        {
            Network_PDP_Context_t context = {
                .apn = PDP_CONTEXT_APN,
                .userName = PDP_CONTEXT_USERNAME,
                .userPasswd = PDP_CONTEXT_PASSWD};
            Network_StartActive(context);
        }
    }
    return true;
}

void NetworkEventDispatch(API_Event_t *pEvent)
{
    switch (pEvent->id)
    {
    case API_EVENT_ID_NETWORK_REGISTER_DENIED:
        Trace(2, "network register denied");
        break;

    case API_EVENT_ID_NETWORK_REGISTER_NO:
        Trace(2, "network register no");
        break;

    case API_EVENT_ID_NETWORK_REGISTERED_HOME:
    case API_EVENT_ID_NETWORK_REGISTERED_ROAMING:
        Trace(2, "network register success");
        AttachActivate();
        break;

    case API_EVENT_ID_NETWORK_DETACHED:
        Trace(2, "network detached");
        AttachActivate();
        break;
    case API_EVENT_ID_NETWORK_ATTACH_FAILED:
        Trace(2, "network attach failed");
        AttachActivate();
        break;

    case API_EVENT_ID_NETWORK_ATTACHED:
        Trace(2, "network attach success");
        AttachActivate();
        break;

    case API_EVENT_ID_NETWORK_DEACTIVED:
        Trace(2, "network deactived");
        AttachActivate();
        break;

    case API_EVENT_ID_NETWORK_ACTIVATE_FAILED:
        Trace(2, "network activate failed");
        AttachActivate();
        break;

    case API_EVENT_ID_NETWORK_ACTIVATED:
        Trace(2, "network activate success..");
        if (semMqttStart)
            OS_ReleaseSemaphore(semMqttStart);
        break;

    case API_EVENT_ID_SIGNAL_QUALITY:
        Trace(2, "CSQ:%d", pEvent->param1);
        break;

    default:
        break;
    }
}


