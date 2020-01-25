#ifndef __REMOVE_RECEIVE_HANDLER_REQUEST_H
#define __REMOVE_RECEIVE_HANDLER_REQUEST_H

#include "../Frame.h"

class RemoveReceiveHandlerRequest : public Frame
{
public:
	// Constructor
	RemoveReceiveHandlerRequest(unsigned char * memory, short handlerId = -1)
		: Frame(FrameType::Synchron, Command::RemoveReceiveHandler, memory)
	{
		WritePayload(0, (unsigned char*)&handlerId, 2);
	}
	RemoveReceiveHandlerRequest(Frame& frame)
		: Frame(frame)
	{
	}

	// Public Methods
	short GetHandlerId()
	{
		short dummy = 0;
		ReadPayload(0, (unsigned char *)&dummy, 2);
		return dummy;
	}
};

#endif