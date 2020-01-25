#ifndef __CONFIGURATION_RESPONSE_H
#define __CONFIGURATION_RESPONSE_H

#include "../Frame.h"

class ConfigurationResponse : public Frame
{
public:
	// Internal Enum
	enum ResponseType
	{
		Success,
		Failure
	};

	// Constructor
	ConfigurationResponse(unsigned char* memory, ResponseType responseType)
		: Frame(FrameType::Synchron, Command::Configure, memory)
	{
		unsigned char responseTypeAsByte = responseType;
		WritePayload(0, (unsigned char *)&responseTypeAsByte, 1);
	}
	ConfigurationResponse(const Frame& frame)
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