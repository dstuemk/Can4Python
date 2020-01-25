rm Teensy40/PacketBuilder.h
cp CanInterface/PacketBuilder.h Teensy40/PacketBuilder.h
rm Teensy40/Common.h
cp CanInterface/Common.h Teensy40/Common.h
rm Teensy40/CanFrame.h
cp CanInterface/CanFrame.h Teensy40/CanFrame.h
rm -rf Teensy40/Frames
cp -R CanInterface/Frames Teensy40/Frames