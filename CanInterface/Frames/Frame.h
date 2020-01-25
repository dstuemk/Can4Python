#ifndef __FRAME_H
#define __FRAME_H

class Frame
{
private:
	// Members
	unsigned char * frame;

	// Private methods
	static void ReadData(int position, unsigned char * dest, int length, unsigned char const * src)
	{
		for (int i = 0; i < length; i++) dest[i] = src[position + i];
	}
	void ReadData(int position, unsigned char * dest, int length)
	{
		ReadData(position, dest, length, frame);
	}
	void WriteData(int position, unsigned char const * src, int length)
	{
		for (int i = 0; i < length; i++) frame[position + i] = src[i];
	}
protected:
	// Protected Constructor
	Frame(unsigned char frameType, unsigned char command, unsigned char * memory)
	{
		frame = memory;
		WriteData(0, (unsigned char*)&frameType, 1);
		WriteData(1, (unsigned char*)&command, 1);
	}

	// Protected Methods
	void WritePayload(int position, unsigned char const * src, int length)
	{
		WriteData(HeaderLength + position, src, length);
	}
	void ReadPayload(int position, unsigned char * dest, int length)
	{
		ReadData(HeaderLength + position, dest, length);
	}
	unsigned char * GetFrame() const
	{
		return frame;
	}
	unsigned char * GetPayload() 
	{
		return frame + HeaderLength;
	}
public:
	// Constants
	static const int FrameSize = 72;
	static const int HeaderLength = 2;

	// Internal enums
	enum FrameType
	{
		Synchron,
		Asynchron
	};
	enum Command
	{
		Configure,
		EnableTransreceiver,
		AddReceiveHandler,
		RemoveReceiveHandler,
		SendFrame,
		FrameReceived
	};
	// Public Constructor
	Frame(const Frame& frame)
	{	
		this->frame = frame.GetFrame();
	}
	Frame(unsigned char* memory)
	{
		frame = memory;
	}
	Frame()
	{
		frame = nullptr;
	}

	// Static methods
	static FrameType GetType(unsigned char frame[])
	{
		unsigned char dummy;
		ReadData(0, (unsigned char *)&dummy, 1, frame);
		return (FrameType)dummy;
	}
	static Command GetCommand(unsigned char frame[])
	{
		unsigned char dummy;
		ReadData(1, (unsigned char *)&dummy, 1, frame);
		return (Command)dummy;
	}
	static int GetFrameSize()
	{
		return FrameSize;
	}
	static int GetPayloadSize()
	{
		return FrameSize - HeaderLength;
	}

	// Public methods
	bool IsValid()
	{
		return nullptr != frame;
	}
	void SetType(FrameType frameType)
	{
		unsigned char dummy = frameType;
		WriteData(0, (unsigned char*)&dummy, 1);
	}
	FrameType GetType()
	{
		return GetType(frame);
	}
	Command GetCommand()
	{
		return GetCommand(frame);
	}
	unsigned char GetData(int index)
	{
		return frame[index];
	}
};

#endif