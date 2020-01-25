#ifndef __ADD_RECEIVE_HANDLER_RESPONSE_H
#define __ADD_RECEIVE_HANDLER_RESPONSE_H

#include "../Frame.h"

class AddReceiveHandlerResponse : public Frame
{
public:
	// Internal Enum
	enum ResponseType
	{
		Success,
		Failure,
		FailureTooManyReceiveHandlers
	};

	// Constructor
	AddReceiveHandlerResponse(unsigned char* memory, ResponseType responseType, unsigned short handlerId = 0)
		: Frame(FrameType::Synchron, Command::AddReceiveHandler, memory)
	{
		unsigned char responseTypeAsByte = responseType;
		WritePayload(0, (unsigned char *)&responseTypeAsByte,	1);
		WritePayload(1, (unsigned char *)&handlerId,			2);
	}
	AddReceiveHandlerResponse(const Frame& frame)
		: Frame(frame)
	{
	}

	// Public Methods
	ResponseType GetResponseType()
	{
		unsigned char dummy = 0;
		ReadPayload(0, (unsigned char *)&dummy, 1);
		return (ResponseType)dummy;
	}
	unsigned int GetHandlerId()
	{
		unsigned int dummy = 0;
		ReadPayload(1, (unsigned char *)&dummy, 2);
		return dummy;
	}
};
#endif