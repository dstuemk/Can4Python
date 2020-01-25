#ifndef __RECEIVE_HANDLER_COMMAND_H
#define __RECEIVE_HANDLER_COMMAND_H

#include "Common.h"
#include <stdint.h>

void InitCanMessageHandlers();
bool ProcessCanMessage(CanFrame& canFrame, Frame& eventFrame, uint8_t* sendBuffer);
bool RemoveReceiveHandler(Frame& requestFrame, Frame& responseFrame, uint8_t* sendBuffer);
bool AddReceiveHandler(Frame& requestFrame, Frame& responseFrame, uint8_t* sendBuffer);

#endif
