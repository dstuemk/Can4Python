#ifndef __CONFIGURATION_REQUEST_H
#define __CONFIGURATION_REQUEST_H

#include "../Frame.h"

class ConfigurationRequest : public Frame
{
public:
	// Internal enum
	enum CanType
	{
		Can20,
		CanFD
	};

	// Constructors
	ConfigurationRequest(unsigned char * memory, bool listenOnly, unsigned long arbitrationBaudrate, unsigned long dataBaudrate,
		double samplePoint = 87.5, double propagationDelay = 190, double busLength = 1) /* CAN FD Configuration */
		: Frame(FrameType::Synchron, Command::Configure, memory)
	{
		unsigned char canType = CanType::CanFD;
		unsigned char listenOnlyAsByte = listenOnly ? 1 : 0;
		WritePayload(0, (unsigned char *)&canType, 1);
		WritePayload(1, (unsigned char *)&listenOnlyAsByte, 1);
		WritePayload(2, (unsigned char *)&arbitrationBaudrate, 4);
		WritePayload(6, (unsigned char *)&dataBaudrate, 4);
		WritePayload(10, (unsigned char *)&samplePoint, 8);
		WritePayload(18, (unsigned char *)&propagationDelay, 8);
		WritePayload(26, (unsigned char *)&busLength, 8);
	}
	ConfigurationRequest(unsigned char * memory, bool listenOnly, unsigned long baudrate) /* CAN 20 Configuration */
		: Frame(FrameType::Synchron, Command::Configure, memory)
	{
		unsigned char canType = CanType::Can20;
		unsigned char listenOnlyAsByte = listenOnly ? 1 : 0;
		WritePayload(0, (unsigned char *)&canType, 1);
		WritePayload(1, (unsigned char *)&listenOnlyAsByte, 1);
		WritePayload(2, (unsigned char *)&baudrate, 4);
	}
	ConfigurationRequest(Frame& frame)
		: Frame(frame)
	{
	}

	// Public Methods
	CanType GetCanType()
	{
		unsigned char dummy = 0;
		ReadPayload(0, (unsigned char *)&dummy, 1);
		return (CanType)dummy;
	}
	bool IsListenOnly()
	{
		unsigned char dummy = 0;
		ReadPayload(1, (unsigned char *)&dummy, 1);
		return dummy != 0;
	}
	unsigned int GetBaudrate()
	{
		unsigned int dummy = 0;
		ReadPayload(2, (unsigned char *)&dummy, 4);
		return dummy;
	}
	unsigned int GetArbitrationBaudrate()
	{
		unsigned int dummy = 0;
		ReadPayload(2, (unsigned char *)&dummy, 4);
		return dummy;
	}
	unsigned int GetDataBaudrate()
	{
		unsigned int dummy = 0;
		ReadPayload(6, (unsigned char *)&dummy, 4);
		return dummy;
	}
	double GetSamplePoint()
	{
		double dummy;
		ReadPayload(10, (unsigned char *)&dummy, 8);
		return dummy;
	}
	double GetPropagationDelay()
	{
		double dummy;
		ReadPayload(18, (unsigned char *)&dummy, 8);
		return dummy;
	}
	double GetBusLength()
	{
		double dummy;
		ReadPayload(26, (unsigned char *)&dummy, 8);
		return dummy;
	}
};

#endif