#ifndef __ADD_RECEIVE_HANDLER_REQUEST_H
#define __ADD_RECEIVE_HANDLER_REQUEST_H

#include "../Frame.h"

class AddReceiveHandlerRequest : public Frame
{
public:
	// Internal enum
	enum FilterType
	{
		Single,
		Range,
		All
	};

	// Constructors
	AddReceiveHandlerRequest(unsigned char * memory) /*Receives all Addresses*/
		: Frame(FrameType::Synchron, Command::AddReceiveHandler, memory)
	{
		unsigned char filterType = FilterType::All;
		WritePayload(0, (unsigned char *)&filterType, 1);
	}
	AddReceiveHandlerRequest(unsigned char* memory, char extendedIdentifier, unsigned long canIdentifier) /* Receives Single Address */
		: Frame(FrameType::Synchron, Command::AddReceiveHandler, memory)
	{
		unsigned char filterType = FilterType::Single;
		WritePayload(0, (unsigned char *)&filterType, 1);
		WritePayload(1, (unsigned char *)&extendedIdentifier, 1);
		WritePayload(2, (unsigned char *)&canIdentifier, 4);
	}
	AddReceiveHandlerRequest(unsigned char* memory, char extendedIdentifier, unsigned long canIdentifierLow, unsigned long canIdentifierHigh) /* Received Address Range */
		: Frame(FrameType::Synchron, Command::AddReceiveHandler, memory)
	{
		unsigned char filterType = FilterType::Range;
		WritePayload(0, (unsigned char *)&filterType, 1);
		WritePayload(1, (unsigned char *)&extendedIdentifier, 1);
		WritePayload(2, (unsigned char *)&canIdentifierLow, 4);
		WritePayload(6, (unsigned char *)&canIdentifierHigh, 4);
	}
	AddReceiveHandlerRequest(Frame& frame)
		: Frame(frame)
	{
	}

	// Public Methods
	FilterType GetFilterType()
	{
		unsigned char dummy = 0;
		ReadPayload(0, (unsigned char *)&dummy, 1);
		return (FilterType)dummy;
	}
	char UseExtendedIdentifiers() /* -1: Dont care | 0: 11 bit Identifier | 1: 29 bit Identifier */
	{
		char dummy = 0;
		ReadPayload(1, (unsigned char *)&dummy, 1);
		return dummy;
	}
	unsigned long GetCanIdentifier()
	{
		unsigned long dummy = 0;
		ReadPayload(2, (unsigned char *)&dummy, 4);
		return dummy;
	}
	unsigned long GetCanIdentifierLow()
	{
		unsigned long dummy = 0;
		ReadPayload(2, (unsigned char *)&dummy, 4);
		return dummy;
	}
	unsigned long GetCanIdentifierHigh()
	{
		unsigned long dummy = 0;
		ReadPayload(6, (unsigned char *)&dummy, 4);
		return dummy;
	}
};

#endif