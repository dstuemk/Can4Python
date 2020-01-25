#ifndef __ENABLE_TRANSRECEIVER_COMMAND_H
#define __ENABLE_TRANSRECEIVER_COMMAND_H

#include "Common.h"
#include <stdint.h>

void InitEnableTransreceiverFunction(bool (*func)(bool));
bool EnableTransreceiver(Frame& requestFrame, Frame& responseFrame, uint8_t* sendBuffer);

#endif
