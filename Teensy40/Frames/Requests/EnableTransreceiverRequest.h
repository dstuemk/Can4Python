#ifndef __ENABLE_TRANSRECEIVER_REQUEST_H
#define __ENABLE_TRANSRECEIVER_REQUEST_H

#include "../Frame.h"

class EnableTransreceiverRequest : public Frame
{
public:
	// Internal enum
	enum EnableState
	{
		Off = 0,
		On = 1
	};

	// Constructors
	EnableTransreceiverRequest(unsigned char* memory, EnableState enableState)
		: Frame(FrameType::Synchron, Command::EnableTransreceiver, memory)
	{
		unsigned char enabled = enableState;
		WritePayload(0, (unsigned char*)&enabled, 1);
	}
	EnableTransreceiverRequest(Frame& frame)
		: Frame(frame)
	{
	}

	// Public Methods
	EnableState GetEnableState()
	{
		unsigned char dummy;
		ReadPayload(0, &dummy, 1);
		return (EnableState)dummy;
	}
};

#endif // __ENABLE_TRANSRECEIVER_REQUEST_H


