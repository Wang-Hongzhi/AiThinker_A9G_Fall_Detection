#ifndef _NETWORK_H
#define _NETWORK_H

#include "api_event.h"

#define PDP_CONTEXT_APN "cmnet"
#define PDP_CONTEXT_USERNAME ""
#define PDP_CONTEXT_PASSWD ""

bool AttachActivate();
void NetworkEventDispatch(API_Event_t *pEvent);

#endif // !_NETWORK_H
