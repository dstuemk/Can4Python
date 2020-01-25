#ifndef __ENABLE_TRANSRECEIVER_RESPONSE_H
#define __ENABLE_TRANSRECEIVER_RESPONSE_H

#include "../Frame.h"

class EnableTransreceiverResponse : public Frame
{
public:
	// Internal Enum
	enum ResponseType
	{
		Success,
		Failure
	};

	// Constructors
	EnableTransreceiverResponse(unsigned char* memory, ResponseType responseType)
		: Frame(FrameType::Synchron, Command::EnableTransreceiver, memory)
	{
		unsigned char responseTypeAsByte = responseType;
		WritePayload(0, (unsigned char*)&responseTypeAsByte, 1);
	}
	EnableTransreceiverResponse(const Frame& frame)
		: Frame(frame)
	{
	}

	// Public Methods
	ResponseType GetResponseType()
	{
		unsigned char dummy = 0;
		ReadPayload(0, (unsigned char*)&dummy, 1);
		return (ResponseType)dummy;
	}
};

#endif // !__ENABLE_TRANSRECEIVER_RESPONSE_H