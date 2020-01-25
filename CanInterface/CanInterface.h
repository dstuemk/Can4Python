#ifndef __CAN_INTERFACE_H
#define __CAN_INTERFACE_H

#include <map>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <condition_variable>
#include <boost/circular_buffer.hpp>

// Forward declarations
class Devices;
class PacketBuilder;

class CanInterface
{
private:
	// Communication Timing
	static const int ResponseTimeout = 100;

	// Private Constants
	const int VendorID	= 0x16C0;
	const int ProductID = 0x0486;
	const int UsagePage = 0xFFAB;
	const int Usage		= 0x0200;

	// Internal Struct
	typedef struct 
	{
		unsigned int identifier;
		unsigned char payload[64];
		int payloadLength;
		int messageFlags;
		unsigned long timestamp;
	} Message;

	// Device Entry Storage
	class DeviceEntry
	{
	private:
		volatile bool isReceiving;
		boost::circular_buffer<Message> messageBuffer;
	public:
		DeviceEntry(int msgBufferSize)
			:	isReceiving(false), messageBuffer(msgBufferSize)
		{}
		~DeviceEntry()
		{
		}
		bool IsReceiving() { return isReceiving; }
		void StartReceiving() { isReceiving = true; }
		void StopReceiving() { isReceiving = false; }
		void AddMessage(Message message)
		{
			if (IsReceiving())
			{
				// Add message to buffer
				messageBuffer.push_back(message);
			}
		}
		bool GetCanMessage(Message& message)
		{
			if (messageBuffer.empty()) return false;

			// Copy message
			message = messageBuffer[0];
			
			// Remove oldest message from buffer
			messageBuffer.pop_front();

			return true;
		}
	};

	// Private Fields
	Devices* devices;
	std::vector<unsigned char*> packetBuffers;
	std::vector<DeviceEntry*> deviceEntries;
	bool stopThreads;
	std::mutex stopThreadsMutex;
	std::thread receiveThread;
	std::thread sendThread;
	int keyCounter;
	std::queue<int> unusedKeys;
	std::map<int, std::function<void(unsigned char*, int)>> receiveHandlers;
	std::mutex receiveHandlersMutex;
	std::vector<PacketBuilder*> sendBuffers;
	std::mutex sendBuffersMutex;
	
	// Private Methods
	void ReceiveHandler();
	void SendHandler();
	bool SendAndReceive(int deviceIndex, unsigned char* requestBuffer, unsigned char* responseBuffer, int command);
	bool SendFrame(unsigned char data[], int devIndex);
	int AddResponseHandler(std::function<void(unsigned char*, int)>);
	void RemoveResponseHandler(int key);
	void CallResponseHandlers(unsigned char* response, int devIndex);

public:
	CanInterface(int msgBufferSize = 2048, int maxDev = 10);
	~CanInterface();
	int GetDeviceCount();
	bool EnableTransreceiver(int devIndex);
	bool DisableTransreceiver(int devIndex);
	bool OpenCan20(int devIndex, unsigned int baudrate, bool listenOnly);
	bool OpenCanFD(int devIndex, unsigned int arbitrationBaudrate, unsigned int dataBaudrate, bool listenOnly,
		double samplePoint, double propDelay, double busLength);
	bool CloseCan(int devIndex);
	void StartReceiving(int devIndex);
	void StopReceiving(int devIndex);
	bool SendCanMessage(int devIndex, unsigned int messageId, unsigned char payload[], int payloadLength,
		int messageFlags);
	bool SendCanMessageAsync(int devIndex, unsigned int messageId, unsigned char payload[], int payloadLength,
		int messageFlags);
	bool GetCanMessage(int devIndex, unsigned int* identifier, unsigned char* payload, int* payloadLength, int* messageFlags, unsigned long* timestamp);
	int AddReceiveHandler(int devIndex, unsigned int messageId, int identifierFlags);
	int AddReceiveHandler(int devIndex, unsigned int lowerMessageId, unsigned int higherMessageId,
		int identifierFlags);
	int AddReceiveHandler(int devIndex, int identifierFlags);
	bool RemoveReceiveHandler(int devIndex, int handlerId = -1);
	
	enum MessageFlags { USE_EDL = 1, USE_BRS = 2, USE_EXT = 4 };
	enum IdentifierFlags { STANDARD_11BIT = 1, EXTENDED_29BIT = 2 };
};

#endif