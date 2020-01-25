# Can4Python
Python API for an Open Source USB to CAN-BUS Interface. Can4Python offers a bunch of functionalities in order to send and receive CAN-Messages.
This implementation uses a [Teensy4.0 Board](https://www.pjrc.com/store/teensy40.html), connected to a [MCP2562FD CAN FD transreceiver](https://www.microchip.com/wwwproducts/en/MCP2562FD) at CAN3 on the hardware side, as well as the [FlexCAN_T4](https://github.com/tonton81/FlexCAN_T4) library in the software side.
In theory also other Controllers could be used with little changes in the main *.ino file, if they are Arduino compatible and offer a CAN-Bus interface. 

## Prerequisites
* Visual Studio 2017 or later with the workloads mentioned in this [article](https://docs.microsoft.com/en-us/visualstudio/python/working-with-c-cpp-python-in-visual-studio?view=vs-2019#create-the-python-application) 
* [Boost 1.65.0](https://www.boost.org/users/history/version_1_65_0.html) and environent variables set to BOOST_ROOT and BOOST_LIBRARYDIR
* Visual C++ 2015.3 v140 toolset installed (via Visual Studio installer → Modify / Individual Components)

## Prepare Hardware
1. Run the script *init-repo.sh*
2. Open the file *Teensy40.ino* with the Arduino Teensy IDE
3. Compile and Flash

## Install the Python package
In order to globally install the Can4Python package, navigate into the *CanInterface* Folder and run `python -m pip install .` in an elevated console.

## Example Script
```python
from Can4Python import CanInterface
import time
can = CanInterface()
can.OpenCan20(0, 500000, False)
can.AddReceiveHandler(0, can.IdentifierFlags.STANDARD_11BIT)
can.StartReceiving(0)
can.SendCanMessage(0, 0x7e0, [0x02, 0x3e, 0x00], 0)
time.sleep(0.5)
resp = can.GetCanMessage(0)
for b in resp['payload']: print(b)
can.CloseCan(0)
```

## CanInterface Documentation
TODO
