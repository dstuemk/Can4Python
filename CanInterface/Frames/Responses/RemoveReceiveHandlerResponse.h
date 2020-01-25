#ifndef __REMOVE_RECEIVE_HANDLER_RESPONSE_H
#define __REMOVE_RECEIVE_HANDLER_RESPONSE_H

#include "../Frame.h"

class RemoveReceiveHandlerResponse : public Frame
{
public:
	// Internal Enum
	enum ResponseType
	{
		Success,
		Failure,
		FailureWrongHandlerId
	};

	// Constructor
	RemoveReceiveHandlerResponse(unsigned char* memory, ResponseType responseType)
		: Frame(FrameType::Synchron, Command::RemoveReceiveHandler, memory)
	{
		unsigned char responseTypeAsByte = responseType;
		WritePayload(0, (unsigned char *)&responseTypeAsByte, 1);
	}
	RemoveReceiveHandlerResponse(const Frame& frame)
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
};

#endif