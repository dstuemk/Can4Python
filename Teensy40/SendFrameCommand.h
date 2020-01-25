#ifndef __SEND_FRAME_COMMAND_H
#define __SEND_FRAME_COMMAND_H

#include "Common.h"
#include <stdint.h>

void InitSendCanFrameFunction(bool(*func)(CanFrame&));
bool SendFrame(Frame& requestFrame, Frame& responseFrame, uint8_t* sendBuffer);

#endif
