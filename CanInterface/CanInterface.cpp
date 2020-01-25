#include "CanInterface.h"
#include "Devices.h"
#include "Common.h"
#include <chrono>

#if DEBUG_LEVEL != 0
#include <iostream>
#endif

CanInterface::CanInterface(int msgBufferSize, int maxDev)
	: keyCounter(0)
{
	stopThreads = false;
	devices = new Devices(VendorID, ProductID, UsagePage, Usage, maxDev);

	int devCount = GetDeviceCount();

	deviceEntries.resize(devCount);	
	sendBuffers.resize(devCount);
	packetBuffers.resize(devCount);
	for (int i = 0; i < devCount; i++)
	{
		deviceEntries[i] = new DeviceEntry(msgBufferSize);
		packetBuffers[i] = new unsigned char[PacketBuilder::GetPacketSize()];
		sendBuffers[i] = new PacketBuilder(packetBuffers[i]);

		// Clear Buffer
		sendBuffers[i]->Flush();
	}

	// Add event received Handler
	AddResponseHandler([this](unsigned char* response, int devIndex) {
		Frame frame(response);
		if (frame.GetType() == Frame::FrameType::Asynchron) {
			switch (frame.GetCommand())
			{
				case Frame::Command::FrameReceived:
				{
					FrameReceivedEvent ev(frame);
					CanFrame canFrame = ev.GetCanFrame();
					Message msg;
					msg.identifier = canFrame.GetIdentifier();
					msg.payloadLength = canFrame.GetPayloadLength();
					msg.timestamp = canFrame.GetTimestamp();
					msg.messageFlags =	(canFrame.ExtendedDataLength() ? MessageFlags::USE_EDL : 0) |
										(canFrame.BitrateSwitch()      ? MessageFlags::USE_BRS : 0) |
										(canFrame.ExtendedIdentifier() ? MessageFlags::USE_EXT : 0);
					for (int i = 0; i < canFrame.GetPayloadLength(); i++) msg.payload[i] = canFrame.GetPayload(i);
					deviceEntries[devIndex]->AddMessage(msg);
				}
				break;
			}
		}
	});

	receiveThread = std::thread(&CanInterface::ReceiveHandler, this);
	sendThread = std::thread(&CanInterface::SendHandler, this);
}

CanInterface::~CanInterface()
{
	// Signal threads to end
	std::unique_lock<std::mutex> stopThreadsLock(stopThreadsMutex);
	stopThreads = true;
	stopThreadsLock.unlock();
	
	receiveThread.join();
	sendThread.join();
	   
	// Free allocated memory
	delete devices;
	for (int i = 0; i < deviceEntries.size(); i++)
	{
		delete deviceEntries[i];
		delete sendBuffers[i];
		delete packetBuffers[i];
	}
}

void CanInterface::SendHandler()
{
	std::vector<std::thread> threads;
	threads.resize(GetDeviceCount());

	for (int devIndex = 0; devIndex < GetDeviceCount(); devIndex++)
	{
		threads[devIndex] = std::thread([this, devIndex]() {
			for (;;)
			{
				std::unique_lock<std::mutex> stopThreadsLock(stopThreadsMutex);
				if (stopThreads)
				{
					#if DEBUG_LEVEL >= 1
					std::cout << "Send Thread " << devIndex << " ended" << std::endl << std::flush;
					#endif	
					break;
				}
				stopThreadsLock.unlock();

				std::unique_lock<std::mutex> sendBuffersLock(sendBuffersMutex);
				if (sendBuffers[devIndex]->GetFrameCount() > 0)
				{
					#if DEBUG_LEVEL >= 2
					std::cout << "Send Buffer: " << sendBuffers[devIndex]->GetFrameCount() << std::endl << std::flush;
					#endif

					// Send Frames and clear buffer
					devices->Send(sendBuffers[devIndex]->GetPacket(), Devices::PacketSize, devIndex);
					sendBuffers[devIndex]->Flush();
				}
				sendBuffersLock.unlock();

				// Wait for 2ms
				std::this_thread::sleep_for(std::chrono::milliseconds(2));
			}
		});
	}

	for (int i = 0; i < threads.size(); i++) threads[i].join();
}

void CanInterface::ReceiveHandler()
{
	std::vector<std::thread> threads;
	threads.resize(GetDeviceCount());

	for (int devIndex = 0; devIndex < GetDeviceCount(); devIndex++)
	{
		threads[devIndex] = std::thread([this, devIndex]() {
			unsigned char packetBuffer[PACKET_SIZE];
			for (;;)
			{

				std::unique_lock<std::mutex> stopThreadsLock(stopThreadsMutex);
				if (stopThreads)
				{
					#if DEBUG_LEVEL >= 1
					std::cout << "Receive Thread " << devIndex << " ended" << std::endl << std::flush;
					#endif	
					break;
				}
				stopThreadsLock.unlock();

				int res;
				res = devices->Receive(packetBuffer, Devices::PacketSize, devIndex, 0);
				if (res > 0)
				{
					PacketBuilder packetBuilder(packetBuffer);
					for (int i = 0; i < packetBuilder.GetFrameCount(); i++)
					{
						unsigned char* frameBuffer = packetBuilder.GetFrameData(i);
						CallResponseHandlers(frameBuffer, devIndex);
					}
			}
			#if DEBUG_LEVEL >= 3
			std::cout << "Receive Thread " << devIndex << " running" << std::endl << std::flush;
			#endif	
			}
		});	
	}

	for (int i = 0; i < threads.size(); i++) threads[i].join();
}

bool CanInterface::SendFrame(unsigned char data[], int devIndex)
{
	for (;;)
	{
		std::unique_lock<std::mutex> lock(sendBuffersMutex);
		if (sendBuffers[devIndex]->AddFrame(data)) break;
		lock.unlock();
		
		#if DEBUG_LEVEL >= 2
		std::cout << "Send Frame with device " << devIndex << " retry" << std::endl << std::flush;
		#endif	
	}
	return true;
}

void CanInterface::CallResponseHandlers(unsigned char* response, int devIndex)
{
	{
		std::unique_lock<std::mutex> lock(receiveHandlersMutex);
		for (auto it = receiveHandlers.begin(); it != receiveHandlers.end(); it++)
		{
			it->second(response, devIndex);
		}
	}
}

void CanInterface::RemoveResponseHandler(int key)
{
	{
		std::unique_lock<std::mutex> lock(receiveHandlersMutex);
		receiveHandlers.erase(key);
	}

	unusedKeys.push(key);
}

int CanInterface::AddResponseHandler(std::function<void(unsigned char*, int)> func)
{
	int key;

	if (!unusedKeys.empty())
	{
		key = unusedKeys.front();
		unusedKeys.pop();
	}
	else
	{
		key = keyCounter++;
	}

	{
		std::unique_lock<std::mutex> lock(receiveHandlersMutex);
		receiveHandlers[key] = func;
	}

	return key;
}

int CanInterface::GetDeviceCount()
{
	return devices->GetDeviceCount();
}

bool CanInterface::SendAndReceive(int deviceIndex, unsigned char* requestBuffer, unsigned char* responseBuffer, int command)
{
	if (deviceIndex < 0 || deviceIndex >= GetDeviceCount()) return false;

	// Setup ResponseHandler
	std::mutex m;
	volatile bool responseReceived = false;
	auto responseHandlerFunction = [&m, &responseReceived, deviceIndex, command, responseBuffer](unsigned char* response, int i) mutable -> void
	{
		if (i != deviceIndex) return;
		
		Frame frame(response);
		if (frame.GetCommand() == command)
		{
			{
				std::unique_lock<std::mutex> lock(m);
				responseReceived = true;
				for (int i = 0; i < frame.GetFrameSize(); i++) responseBuffer[i] = response[i];
			}
		}
	};

	int key = AddResponseHandler(responseHandlerFunction);

	bool success;
	if (!SendFrame(requestBuffer, deviceIndex))
	{
		success = false;
		#if DEBUG_LEVEL == 1
		std::cout << "# SendAndReceive: SendFrame failed, Device " << deviceIndex << std::endl << std::flush;
		#endif	
	}
	else if (Frame(requestBuffer).GetType() == Frame::FrameType::Asynchron)
	{
		// Don´t wait for response
		success = true;
	}
	else
	{
		// Wait for response or timeout
		auto startTime = std::chrono::high_resolution_clock::now();
		for (;;)
		{
			{
				std::unique_lock<std::mutex> lock(m);
				if (responseReceived)
				{
					success = true;
					break;
				}
			}

			auto currTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> elapsedTime = currTime - startTime;

			if (elapsedTime.count() >= ResponseTimeout)
			{
				#if DEBUG_LEVEL == 1
				std::cout << "# SendAndReceive: Timeout, Device " << deviceIndex << std::endl << std::flush;
				#endif			
				// Timeout
				success = false;
				break;
			}

			// Wait for 1ms
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	RemoveResponseHandler(key);

	return success;
}

bool CanInterface::EnableTransreceiver(int devIndex)
{
	unsigned char sendBuffer[Frame::FrameSize];
	unsigned char recvBuffer[Frame::FrameSize];

	EnableTransreceiverRequest req(sendBuffer, EnableTransreceiverRequest::EnableState::On);

	bool result = SendAndReceive(devIndex, sendBuffer, recvBuffer, req.GetCommand());

	if (result)
	{
		EnableTransreceiverResponse resp = EnableTransreceiverResponse(Frame(recvBuffer));
		if (resp.GetResponseType() == EnableTransreceiverResponse::ResponseType::Success) return true;
	}

	return false;
}

bool CanInterface::DisableTransreceiver(int devIndex)
{
	unsigned char sendBuffer[Frame::FrameSize];
	unsigned char recvBuffer[Frame::FrameSize];

	EnableTransreceiverRequest req(sendBuffer, EnableTransreceiverRequest::EnableState::Off);

	bool result = SendAndReceive(devIndex, sendBuffer, recvBuffer, req.GetCommand());

	if (result)
	{
		EnableTransreceiverResponse resp = EnableTransreceiverResponse(Frame(recvBuffer));
		if (resp.GetResponseType() == EnableTransreceiverResponse::ResponseType::Success) return true;
	}

	return false;
}

bool CanInterface::OpenCan20(int devIndex, unsigned int baudrate, bool listenOnly)
{
	if (!EnableTransreceiver(devIndex)) return false;

	unsigned char sendBuffer[Frame::FrameSize];
	unsigned char recvBuffer[Frame::FrameSize];

	ConfigurationRequest req(sendBuffer, listenOnly, baudrate);

	bool result = SendAndReceive(devIndex, sendBuffer, recvBuffer, req.GetCommand());

	if (result)
	{
		ConfigurationResponse resp = ConfigurationResponse(Frame(recvBuffer));
		if (resp.GetResponseType() == ConfigurationResponse::ResponseType::Success) return true;
	}

	return false;
}

bool CanInterface::OpenCanFD(int devIndex, unsigned int arbitrationBaudrate, unsigned int dataBaudrate, bool listenOnly,
	double samplePoint, double propDelay, double busLength)
{
	if (!EnableTransreceiver(devIndex)) return false;

	unsigned char sendBuffer[Frame::FrameSize];
	unsigned char recvBuffer[Frame::FrameSize];
	ConfigurationRequest req(sendBuffer, listenOnly, arbitrationBaudrate, dataBaudrate, samplePoint, propDelay, busLength);

	bool result = SendAndReceive(devIndex, sendBuffer, recvBuffer, req.GetCommand());

	if (result)
	{
		ConfigurationResponse resp = ConfigurationResponse(Frame(recvBuffer));
		if (resp.GetResponseType() == ConfigurationResponse::ResponseType::Success) return true;
	}

	return false;
}

bool CanInterface::CloseCan(int devIndex)
{
	if (!DisableTransreceiver(devIndex)) return false;

	unsigned char sendBuffer[Frame::FrameSize];
	unsigned char recvBuffer[Frame::FrameSize];
	RemoveReceiveHandlerRequest req(sendBuffer);

	bool result = SendAndReceive(devIndex, sendBuffer, recvBuffer, req.GetCommand());

	if (result)
	{
		RemoveReceiveHandlerResponse resp = RemoveReceiveHandlerResponse(Frame(recvBuffer));
		if (resp.GetResponseType() == RemoveReceiveHandlerResponse::ResponseType::Success) return true;
	}

	return false;
}

void CanInterface::StartReceiving(int devIndex)
{
	if (devIndex < 0 || devIndex >= GetDeviceCount()) return;

	deviceEntries[devIndex]->StartReceiving();
}

void CanInterface::StopReceiving(int devIndex)
{
	if (devIndex < 0 || devIndex >= GetDeviceCount()) return;

	deviceEntries[devIndex]->StopReceiving();
}

bool CanInterface::SendCanMessageAsync(int devIndex, unsigned int messageId, unsigned char payload[], int payloadLength,
	int messageFlags)
{
	unsigned char sendBuffer[Frame::FrameSize];
	unsigned char recvBuffer[Frame::FrameSize];
	SendFrameRequest req(sendBuffer, messageId,
		(messageFlags & MessageFlags::USE_EXT) != 0,
		(messageFlags & MessageFlags::USE_EDL) != 0,
		(messageFlags & MessageFlags::USE_BRS) != 0,
		payload, payloadLength);
	req.SetType(Frame::FrameType::Asynchron);

	bool result = SendAndReceive(devIndex, sendBuffer, recvBuffer, req.GetCommand());
	return result;
}

bool CanInterface::SendCanMessage(int devIndex, unsigned int messageId, unsigned char payload[], int payloadLength,
	int messageFlags)
{
	unsigned char sendBuffer[Frame::FrameSize];
	unsigned char recvBuffer[Frame::FrameSize];
	SendFrameRequest req(sendBuffer, messageId,
		(messageFlags & MessageFlags::USE_EXT) != 0,
		(messageFlags & MessageFlags::USE_EDL) != 0,
		(messageFlags & MessageFlags::USE_BRS) != 0,
		payload, payloadLength);

	bool result = SendAndReceive(devIndex, sendBuffer, recvBuffer, req.GetCommand());

	if (result)
	{
		SendFrameResponse resp = SendFrameResponse(Frame(recvBuffer));
		if (resp.GetResponseType() == SendFrameResponse::ResponseType::Success) return true;
	}

	return false;
}

bool CanInterface::GetCanMessage(int devIndex, unsigned int* identifier, unsigned char* payload, int* payloadLength, int* messageFlags, unsigned long* timestamp)
{
	if (devIndex < 0 || devIndex >= GetDeviceCount()) return false;

	Message msg;
	if (!deviceEntries[devIndex]->GetCanMessage(msg)) return false;

	*identifier = msg.identifier;
	*payloadLength = msg.payloadLength;
	*messageFlags = msg.messageFlags;
	*timestamp = msg.timestamp;
	for (int i = 0; i < *payloadLength; i++) payload[i] = msg.payload[i];

	return true;
}

int CanInterface::AddReceiveHandler(int devIndex, unsigned int messageId, int identifierFlags)
{
	unsigned char sendBuffer[Frame::FrameSize];
	unsigned char recvBuffer[Frame::FrameSize];
	char extIdentifier;
	if ((identifierFlags & IdentifierFlags::STANDARD_11BIT) != 0 && (identifierFlags & IdentifierFlags::EXTENDED_29BIT) != 0) extIdentifier = -1;	// Receive all
	if ((identifierFlags & IdentifierFlags::STANDARD_11BIT) != 0 && (identifierFlags & IdentifierFlags::EXTENDED_29BIT) == 0) extIdentifier = 0;	// Only 11 Bit
	if ((identifierFlags & IdentifierFlags::STANDARD_11BIT) == 0 && (identifierFlags & IdentifierFlags::EXTENDED_29BIT) != 0) extIdentifier = 1;	// Only 29 Bit
	if ((identifierFlags & IdentifierFlags::STANDARD_11BIT) == 0 && (identifierFlags & IdentifierFlags::EXTENDED_29BIT) == 0) return -1;			// Invalid Flags
	AddReceiveHandlerRequest req(sendBuffer, extIdentifier, messageId);

	bool result = SendAndReceive(devIndex, sendBuffer, recvBuffer, req.GetCommand());

	if (result)
	{
		AddReceiveHandlerResponse resp = AddReceiveHandlerResponse(Frame(recvBuffer));
		if (resp.GetResponseType() == AddReceiveHandlerResponse::ResponseType::Success) return resp.GetHandlerId();
	}

	return -1;
}

int CanInterface::AddReceiveHandler(int devIndex, unsigned int lowerMessageId, unsigned int higherMessageId,
	int identifierFlags)
{
	unsigned char sendBuffer[Frame::FrameSize];
	unsigned char recvBuffer[Frame::FrameSize];
	char extIdentifier;
	if ((identifierFlags & IdentifierFlags::STANDARD_11BIT) != 0 && (identifierFlags & IdentifierFlags::EXTENDED_29BIT) != 0) extIdentifier = -1;	// Receive all
	if ((identifierFlags & IdentifierFlags::STANDARD_11BIT) != 0 && (identifierFlags & IdentifierFlags::EXTENDED_29BIT) == 0) extIdentifier = 0;	// Only 11 Bit
	if ((identifierFlags & IdentifierFlags::STANDARD_11BIT) == 0 && (identifierFlags & IdentifierFlags::EXTENDED_29BIT) != 0) extIdentifier = 1;	// Only 29 Bit
	if ((identifierFlags & IdentifierFlags::STANDARD_11BIT) == 0 && (identifierFlags & IdentifierFlags::EXTENDED_29BIT) == 0) return -1;			// Invalid Flags
	AddReceiveHandlerRequest req(sendBuffer, extIdentifier, lowerMessageId, higherMessageId);

	bool result = SendAndReceive(devIndex, sendBuffer, recvBuffer, req.GetCommand());

	if (result)
	{
		AddReceiveHandlerResponse resp = AddReceiveHandlerResponse(Frame(recvBuffer));
		if (resp.GetResponseType() == AddReceiveHandlerResponse::ResponseType::Success) return resp.GetHandlerId();
	}

	return -1;
}

int CanInterface::AddReceiveHandler(int devIndex, int identifierFlags)
{
	unsigned char sendBuffer[Frame::FrameSize];
	unsigned char recvBuffer[Frame::FrameSize];
	char extIdentifier;
	if ((identifierFlags & IdentifierFlags::STANDARD_11BIT) != 0 && (identifierFlags & IdentifierFlags::EXTENDED_29BIT) != 0) extIdentifier = -1;	// Receive all
	if ((identifierFlags & IdentifierFlags::STANDARD_11BIT) != 0 && (identifierFlags & IdentifierFlags::EXTENDED_29BIT) == 0) extIdentifier = 0;	// Only 11 Bit
	if ((identifierFlags & IdentifierFlags::STANDARD_11BIT) == 0 && (identifierFlags & IdentifierFlags::EXTENDED_29BIT) != 0) extIdentifier = 1;	// Only 29 Bit
	if ((identifierFlags & IdentifierFlags::STANDARD_11BIT) == 0 && (identifierFlags & IdentifierFlags::EXTENDED_29BIT) == 0) return -1;			// Invalid Flags
	AddReceiveHandlerRequest req(sendBuffer, extIdentifier, 0x00000000, 0xFFFFFFFF);

	bool result = SendAndReceive(devIndex, sendBuffer, recvBuffer, req.GetCommand());

	if (result)
	{
		AddReceiveHandlerResponse resp = AddReceiveHandlerResponse(Frame(recvBuffer));
		if (resp.GetResponseType() == AddReceiveHandlerResponse::ResponseType::Success) return resp.GetHandlerId();
	}

	return -1;
}

bool CanInterface::RemoveReceiveHandler(int devIndex, int handlerId)
{
	unsigned char sendBuffer[Frame::FrameSize];
	unsigned char recvBuffer[Frame::FrameSize];
	RemoveReceiveHandlerRequest req(sendBuffer, handlerId);

	bool result = SendAndReceive(devIndex, sendBuffer, recvBuffer, req.GetCommand());

	if (result)
	{
		RemoveReceiveHandlerResponse resp = RemoveReceiveHandlerResponse(Frame(recvBuffer));
		if (resp.GetResponseType() == RemoveReceiveHandlerResponse::ResponseType::Success) return true;
	}

	return false;
}
