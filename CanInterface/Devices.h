#ifndef __DEVICES_H
#define __DEVICES_H

#include <string>
#include <vector>
#include "Common.h"
#include "hidapi/hidapi.h"

class Devices
{
private:
	std::vector<hid_device*> devices;
public:
	// Public constants
	static const int PacketSize = PACKET_SIZE;

	// Constructor
	Devices(int vendorID, int productID, int usagePage, int usage, int maxDevices = 10);

	// Destructor
	~Devices();

	// Public Methods
	int GetDeviceCount();
	int Send(void* src, int dataSize, int deviceIndex);
	int Receive(void* dest, int dataSize, int deviceIndex, int timeout = -1);
};

#endif
