#ifndef __CONFIGURE_COMMAND_H
#define __CONFIGURE_COMMAND_H

#include "Common.h"
#include <stdint.h>

void InitConfigureFunction(bool (*func)(ConfigurationRequest&));
bool Configure(Frame& requestFrame, Frame& responseFrame, uint8_t* sendBuffer);

#endif
