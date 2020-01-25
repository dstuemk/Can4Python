#ifndef __FRAME_RECEIVED_EVENT_H
#define __FRAME_RECEIVED_EVENT_H

#include "../Frame.h"
#include "../../CanFrame.h"

class FrameReceivedEvent : public Frame
{
public:
	// Constructor
	FrameReceivedEvent(unsigned char* memory, CanFrame& canFrame)
		: Frame(FrameType::Asynchron, Command::FrameReceived, memory)
	{
		unsigned long timestamp = canFrame.GetTimestamp();
		unsigned long identifier = canFrame.GetIdentifier();
		unsigned char bitValues =	(canFrame.ExtendedIdentifier()	? (1 << 0) : 0)	|
									(canFrame.ExtendedDataLength()	? (1 << 1) : 0)	|
									(canFrame.BitrateSwitch()		? (1 << 2) : 0);
		unsigned char payloadLength = canFrame.GetPayloadLength();
		unsigned char const * payload = canFrame.GetPayload();

		WritePayload(0, (unsigned char *)&timestamp,		4);
		WritePayload(4, (unsigned char *)&identifier,		4);
		WritePayload(8, (unsigned char *)&bitValues,		1);
		WritePayload(9, (unsigned char *)&payloadLength,	1);
		WritePayload(10, (unsigned char *)payload, payloadLength);
	}
	FrameReceivedEvent(Frame& frame)
		: Frame(frame)
	{
	}

	// Public Methods
	CanFrame GetCanFrame()
	{
		unsigned long timestamp;
		unsigned long identifier;
		unsigned char bitValues;
		unsigned char payloadLength;
		unsigned char payload[64];

		ReadPayload(0, (unsigned char *)&timestamp, 4);
		ReadPayload(4, (unsigned char *)&identifier, 4);
		ReadPayload(8, (unsigned char *)&bitValues, 1);
		ReadPayload(9, (unsigned char *)&payloadLength, 1);
		ReadPayload(10, payload, payloadLength);

		bool ext = (bitValues & (1 << 0)) != 0;
		bool edl = (bitValues & (1 << 1)) != 0;
		bool brs = (bitValues & (1 << 2)) != 0;

		return CanFrame(timestamp, identifier, ext, edl, brs, payload, payloadLength);
	}
};

#endif