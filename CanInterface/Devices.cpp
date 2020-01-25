#include "Devices.h"
#include <thread>

#if DEBUG_LEVEL != 0
#include <iostream>
#endif

Devices::Devices(int vendorID, int productID, int usagePage, int usage, int maxDevices)
{
	int ret = hid_init();

	hid_device_info* deviceList = hid_enumerate(vendorID, productID);

	while (deviceList && devices.size() < maxDevices)
	{
		hid_device_info* deviceInfo = deviceList;

		if (deviceInfo->usage_page == usagePage && deviceInfo->usage == usage)
		{
			hid_device* device = hid_open_path(deviceInfo->path);
			devices.push_back(device);
		}

		deviceList = deviceInfo->next;
	} 

	hid_free_enumeration(deviceList);
}

Devices::~Devices()
{
	for (int i = 0; i < devices.size(); i++)
	{
		hid_close(devices[i]);
	}
	hid_exit();
}

int Devices::GetDeviceCount() 
{ 
	return devices.size();
}

#include <iostream>
int Devices::Send(void* src, int dataSize, int deviceIndex)
{
	unsigned char buffer[PacketSize + 1];
	buffer[0] = 0;

	if (dataSize > PacketSize) return -1;

	for (int i = 0; i < PacketSize; i++) buffer[i + 1] = ((unsigned char*)src)[i];
	
	int res = hid_write(devices[deviceIndex], buffer, PacketSize + 1);

	if (res == -1) return -1;

	return dataSize;
}

int Devices::Receive(void* dest, int dataSize, int deviceIndex, int timeout)
{
	unsigned char buffer[PacketSize + 1];

	if (dataSize > PacketSize) return -1;

	int res = hid_read_timeout(devices[deviceIndex], buffer, PacketSize + 1, timeout);
	std::this_thread::sleep_for(std::chrono::milliseconds(1));

	if (res < 1) return res;
	

	for (int i = 0; i < dataSize; i++) ((unsigned char*)dest)[i] = buffer[i];

	return dataSize;
}