#ifndef __SEND_FRAME_REQUEST_H
#define __SEND_FRAME_REQUEST_H

#include "../Frame.h"
#include "../../CanFrame.h"

class SendFrameRequest : public Frame
{
private:
public:
	// Constructor
	SendFrameRequest(unsigned char * memory, unsigned long canIdentifier, bool useExtendedIdentifier, bool useExtendedDataLength, bool useBitrateSwitch, unsigned char const * payload, unsigned char payloadLength)
		: Frame(FrameType::Synchron, Command::SendFrame, memory)
	{
		unsigned char bitValues =	(useExtendedIdentifier	? (1 << 0) : 0) |
									(useExtendedDataLength	? (1 << 1) : 0) |
									(useBitrateSwitch		? (1 << 2) : 0);
		WritePayload(0, (unsigned char*)&canIdentifier, 4);
		WritePayload(4, (unsigned char*)&bitValues,		1);
		WritePayload(5, (unsigned char*)&payloadLength, 1);
		WritePayload(6, payload, payloadLength);
	}
	SendFrameRequest(unsigned char * memory, CanFrame& canFrame)
		: SendFrameRequest(memory, canFrame.GetIdentifier(), canFrame.ExtendedIdentifier(), canFrame.ExtendedDataLength(), canFrame.BitrateSwitch(), canFrame.GetPayload(), canFrame.GetPayloadLength())
	{
	}
	SendFrameRequest(Frame& frame)
		: Frame(frame)
	{
	}

	// Public Methods
	CanFrame GetCanFrame()
	{
		unsigned char payload[64];
		GetPayload(payload);
		return CanFrame(0, GetCanIdentifier(), UseExtendedIdentifier(), UseExtendedDataLength(), UseBitrateSwitch(), payload, GetPayloadLength());
	}
	unsigned long GetCanIdentifier()
	{
		unsigned long dummy = 0;
		ReadPayload(0, (unsigned char *)&dummy, 4);
		return dummy;
	}
	bool UseExtendedIdentifier()
	{
		unsigned char dummy = 0;
		ReadPayload(4, (unsigned char *)&dummy, 1);
		return (dummy & (1 << 0)) != 0;
	}
	bool UseExtendedDataLength()
	{
		unsigned char dummy = 0;
		ReadPayload(4, (unsigned char*)&dummy, 1);
		return (dummy & (1 << 1)) != 0;
	}
	bool UseBitrateSwitch()
	{
		unsigned char dummy = 0;
		ReadPayload(4, (unsigned char *)&dummy, 1);
		return (dummy & (1 << 2)) != 0;
	}
	unsigned char GetPayloadLength()
	{
		unsigned char dummy = 0;
		ReadPayload(5, (unsigned char *)&dummy, 1);
		return dummy;
	}
	void GetPayload(unsigned char * buffer)
	{
		ReadPayload(6, buffer, GetPayloadLength());
	}
};

#endif