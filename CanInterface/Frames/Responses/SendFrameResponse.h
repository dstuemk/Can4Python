#ifndef __SEND_FRAME_RESPONSE_H
#define __SEND_FRAME_RESPONSE_H

#include "../Frame.h"

class SendFrameResponse : public Frame
{
public:
	// Internal Enum
	enum ResponseType
	{
		Success,
		Failure,
		FailureTxQueueFull
	};

	// Constructor
	SendFrameResponse(unsigned char* memory, ResponseType responseType)
		: Frame(FrameType::Synchron, Command::SendFrame, memory)
	{
		unsigned char responseTypeAsByte = responseType;
		WritePayload(0, (unsigned char *)&responseTypeAsByte, 1);
	}
	SendFrameResponse(const Frame& frame)
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