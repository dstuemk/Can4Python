#ifndef __PACKET_BUILDER_H
#define __PACKET_BUILDER_H

#include "Common.h"

class PacketBuilder
{
private:
	// Static constants
	static const int PacketSize = PACKET_SIZE;

	// Member variables
	int			   writePosition;
	unsigned char* packet;

	// Private Methods
	void WriteByte(unsigned char value)
	{
		packet[writePosition] = value;
		writePosition += 1;
	}
	void IncreaseFrameCounter()
	{
		packet[0] += 1;
	}
public:
	// Constructor
	PacketBuilder(unsigned char* packet)
	{
		this->packet = packet;
		writePosition = 1;
	}
	// Public Methods
	void Flush()
	{
		writePosition = 0;

		// SetFrameCounter to 0
		WriteByte(0);
	}
	bool AddFrame(Frame frame)
	{
		if (GetRemainingBytes() > frame.GetFrameSize() + 1)
		{
			WriteByte(frame.GetFrameSize());
			for (int i = 0; i < frame.GetFrameSize(); i++)
			{
				WriteByte(frame.GetData(i));
			}
			IncreaseFrameCounter();
			return true;
		}
		return false;
	}
	int GetFrameCount()
	{
		return packet[0];
	}
	int GetRemainingBytes()
	{
		return PacketSize - writePosition;
	}
	unsigned char* GetFrameData(int index)
	{
		int currentFrameIndex = 0;
		int readPosition = 1;

		while (currentFrameIndex < index)
		{
			readPosition += packet[readPosition] + 1; 
			currentFrameIndex += 1;
		}

		return &packet[readPosition + 1];
	}
	Frame GetFrame(int index)
	{
		Frame frame(GetFrameData(index));
		return frame;
	}
	unsigned char* GetPacket()
	{
		return packet;
	}
	static int GetPacketSize()
	{
		return PacketSize;
	}
};

#endif