#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <Windows.h>
#include <cmath>
#include "CanInterface.h"

using namespace pybind11::literals;
namespace py = pybind11;

class Can4Python : public CanInterface
{
public:
	Can4Python()
		: CanInterface()
	{}

	bool SendCanMessage(int devIndex, unsigned int messageId, std::vector<unsigned char> payload, int messageFlags)
	{
		unsigned char payloadArr[64];
		for (int i = 0; i < payload.size(); i++) payloadArr[i] = payload.at(i);
		return CanInterface::SendCanMessage(devIndex, messageId, payloadArr, payload.size(), messageFlags);
	}
	bool SendCanMessageAsync(int devIndex, unsigned int messageId, std::vector<unsigned char> payload, int payloadLength,
		int messageFlags)
	{
		unsigned char payloadArr[64];
		for (int i = 0; i < payload.size(); i++) payloadArr[i] = payload.at(i);
		return CanInterface::SendCanMessageAsync(devIndex, messageId, payloadArr, payload.size(), messageFlags);
	}

	py::dict GetCanMessage(int devIndex)
	{
		unsigned int identifier;
		unsigned char payload[64];
		int payloadLength;
		int messageFlags;
		unsigned long timestamp;

		if (CanInterface::GetCanMessage(devIndex, &identifier, payload, &payloadLength, &messageFlags, &timestamp))
		{
			auto payloadArr = std::vector<unsigned char>(); 
			for (int i = 0; i < payloadLength; i++) payloadArr.push_back(payload[i]);
			return py::dict("identifier"_a = identifier, "payload"_a = payloadArr, "messageFlags"_a = messageFlags, "timestamp"_a = timestamp);
		}

		return py::dict();
	}
};

PYBIND11_MODULE(Can4Python, mod) {
	py::class_<Can4Python, std::shared_ptr<Can4Python>> clsCan4Python(mod, "CanInterface");

	py::enum_<Can4Python::IdentifierFlags>(clsCan4Python, "IdentifierFlags")
		.value("EXTENDED_29BIT", Can4Python::IdentifierFlags::EXTENDED_29BIT)
		.value("STANDARD_11BIT", Can4Python::IdentifierFlags::STANDARD_11BIT);

	py::enum_<Can4Python::MessageFlags>(clsCan4Python, "MessageFlags")
		.value("USE_BRS", Can4Python::MessageFlags::USE_BRS)
		.value("USE_EDL", Can4Python::MessageFlags::USE_EDL)
		.value("USE_EXT", Can4Python::MessageFlags::USE_EXT);
	
	clsCan4Python.def(py::init<>());
	clsCan4Python.def("GetDeviceCount", &Can4Python::GetDeviceCount);
	clsCan4Python.def("EnableTransreceiver", &Can4Python::EnableTransreceiver);
	clsCan4Python.def("DisableTransreceiver", &Can4Python::DisableTransreceiver);
	clsCan4Python.def("OpenCan20", &Can4Python::OpenCan20);
	clsCan4Python.def("OpenCanFD", &Can4Python::OpenCanFD);
	clsCan4Python.def("CloseCan", &Can4Python::CloseCan);
	clsCan4Python.def("StartReceiving", &Can4Python::StartReceiving);
	clsCan4Python.def("StopReceiving", &Can4Python::StopReceiving);
	clsCan4Python.def("SendCanMessage", &Can4Python::SendCanMessage);
	clsCan4Python.def("SendCanMessageAsync", &Can4Python::SendCanMessageAsync);
	clsCan4Python.def("GetCanMessage", &Can4Python::GetCanMessage);
	clsCan4Python.def("AddReceiveHandler",
		(int (Can4Python::*)(int, unsigned int, int) ) & Can4Python::AddReceiveHandler,
		"devIndex"_a, "messageId"_a, "identifierFlags"_a);
	clsCan4Python.def("AddReceiveHandler",
		(int (Can4Python::*)(int, unsigned int, unsigned int, int)) & Can4Python::AddReceiveHandler,
		"devIndex"_a, "lowerMessageId"_a, "higherMessageId"_a, "identifierFlags"_a);
	clsCan4Python.def("AddReceiveHandler",
		(int (Can4Python::*)(int, int)) & Can4Python::AddReceiveHandler,
		"devIndex"_a, "identifierFlags"_a);
	clsCan4Python.def("RemoveReceiveHandler", &Can4Python::RemoveReceiveHandler, "devIndex"_a, "handlerId"_a = -1);
	
#ifdef VERSION_INFO
	mod.attr("__version__") = VERSION_INFO;
#else
	mod.attr("__version__") = "dev";
#endif
}