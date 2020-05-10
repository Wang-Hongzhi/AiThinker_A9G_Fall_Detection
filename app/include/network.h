#ifndef _NETWORK_H
#define _NETWORK_H

#include "api_event.h"

#define PDP_CONTEXT_APN "cmnet"
#define PDP_CONTEXT_USERNAME ""
#define PDP_CONTEXT_PASSWD ""

extern HANDLE semNetworkSuccess;

bool AttachActivate();
void Network_EventDispatch(API_Event_t *pEvent);

#endif // !_NETWORK_H
