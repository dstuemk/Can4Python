#ifndef __CAN_FRAME_H
#define __CAN_FRAME_H

class CanFrame
{
private:
	unsigned long timestamp;
	unsigned long identifier;
	bool extendedIdentifier;
	bool extendedDataLength;
	bool bitrateSwitch;
	unsigned char payloadLength;
	unsigned char payload[64];
public:
	CanFrame(unsigned long timestamp, unsigned long identifier, bool extendedIdentifier,
		bool extendedDataLength, bool bitrateSwitch, unsigned char payload[], unsigned char payloadLength)
	{
		this->timestamp = timestamp;
		this->identifier = identifier;
		this->extendedIdentifier = extendedIdentifier;
		this->extendedDataLength = extendedDataLength;
		this->bitrateSwitch = bitrateSwitch;
		this->payloadLength = payloadLength;

		for (int i = 0; i < payloadLength; i++)
		{
			this->payload[i] = payload[i];
		}
	}
	unsigned long GetTimestamp() { return timestamp; }
	unsigned long GetIdentifier() { return identifier; }
	bool ExtendedIdentifier() { return extendedIdentifier; }
	bool ExtendedDataLength() { return extendedDataLength; }
	bool BitrateSwitch() { return bitrateSwitch; }
	unsigned char GetPayloadLength() { return payloadLength; }
	unsigned char* GetPayload() { return payload; }
	unsigned char GetPayload(int index) { return payload[index]; }
};

#endif