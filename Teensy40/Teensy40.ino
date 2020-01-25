#include "Common.h"
#include "SendFrameCommand.h"
#include "ReceiveHandlerCommand.h"
#include "EnableTransreceiverCommand.h"
#include "ConfigureCommand.h"

/******************************************************************************
 * BEGIN RAWHID Settings Section
 */
#define VENDOR_ID               0x16C0
#define RAWHID_USAGE_PAGE       0xFFAB  // recommended: 0xFF00 to 0xFFFF
#define RAWHID_USAGE            0x0200  // recommended: 0x0100 to 0xFFFF
/*
 * END RAWHID Settings Section
 ******************************************************************************/
 
/******************************************************************************
 * BEGIN Main Program Section
 */
#include <FlexCAN_T4.h>

typedef bool (*RequestHandler)(Frame&, Frame&, uint8_t*);

RequestHandler requestHandlers[] = 
{
  SendFrame,
  AddReceiveHandler,
  RemoveReceiveHandler,
  Configure,
  EnableTransreceiver
};

const int PacketSize = PACKET_SIZE;

unsigned char sendPacketBuffer[PacketSize];
unsigned char recvPacketBuffer[PacketSize];

#define TRANSRECEIVER_EN_PIN 12 
#define LED_PIN 13
uint16_t ledPeriod = 500;

FlexCAN_T4FD<CAN3, RX_SIZE_256, TX_SIZE_16> flexCan;

void setup() {
  pinMode(TRANSRECEIVER_EN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);  

  // Initialize CAN
  flexCan.begin();
  CANFD_timings_t defaultConfig;
  defaultConfig.baudrate = 500000;
  flexCan.setRegions(8);
  flexCan.setBaudRate(defaultConfig, LISTEN_ONLY);
  
  auto senderFunction = [](CanFrame& canFrame) -> bool
  {
    CANFD_message_t message;
    message.id = canFrame.GetIdentifier();
    message.brs = canFrame.BitrateSwitch();
    message.edl = canFrame.ExtendedDataLength();
    message.flags.extended = canFrame.ExtendedIdentifier();    
    message.len = canFrame.GetPayloadLength();
    
    for(int i = 0; i < message.len; i++)
    {
      message.buf[i] = canFrame.GetPayload(i);
    }
    
    return flexCan.write(message);
  };

  auto configurationFunction = [](ConfigurationRequest& request) -> bool
  {
    CANFD_timings_t config;
      
    switch(request.GetCanType())
    {
      case ConfigurationRequest::CanType::Can20:
      config.baudrate = request.GetBaudrate();
      flexCan.setRegions(8);
      flexCan.setBaudRate(config, request.IsListenOnly() ? LISTEN_ONLY : TX);
      break;
      
      case ConfigurationRequest::CanType::CanFD:
      config.clock = CLK_24MHz;
      config.baudrate = request.GetArbitrationBaudrate();
      config.baudrateFD = request.GetDataBaudrate();
      config.propdelay = request.GetPropagationDelay();
      config.bus_length = request.GetBusLength();
      config.sample = request.GetSamplePoint();
      flexCan.setRegions(64);
      flexCan.setBaudRate(config, request.IsListenOnly() ? LISTEN_ONLY : TX);
      break;

      default:
      return false;
    }

    return true;
  };

  auto enableTransreceiverFunction = [](bool b) -> bool 
  {
    if(b)
    {
      ledPeriod = 250;
      digitalWrite(TRANSRECEIVER_EN_PIN, 1);
    }
    else  
    {
      ledPeriod = 500;
      digitalWrite(TRANSRECEIVER_EN_PIN, 0);
    }
    return true;
  };
  
  InitCanMessageHandlers();
  InitSendCanFrameFunction(senderFunction);  
  InitConfigureFunction(configurationFunction);
  InitEnableTransreceiverFunction(enableTransreceiverFunction);
}

void loop() {
  // Receive Requests
  UpdateReceive();

  // Set up PacketBuilder for sending Frames, calling Flush() to clear Buffer
  PacketBuilder sendPacketBuilder(sendPacketBuffer);
  sendPacketBuilder.Flush();
  // Receive CAN Messages for max. 10 ms
  CANFD_message_t message;
  long lastTimestamp = millis();
  while(flexCan.read(message) && millis() - lastTimestamp < 10)
  {
    CanFrame canFrame(millis(), message.id, message.flags.extended, message.edl, message.brs, message.buf, message.len);
    Frame event;
    byte sendBuffer[Frame::FrameSize];
    if(ProcessCanMessage(canFrame, event, sendBuffer))
    {
      if(!sendPacketBuilder.AddFrame(event)) 
      {
        // If Send Buffer is full, send Packet
        if(RawHID.send(sendPacketBuffer, 10) <= 0) ledPeriod = 100;

        // Clear Buffer and add packet which did not fit in anymore
        sendPacketBuilder.Flush();
        sendPacketBuilder.AddFrame(event);
      }
    }
  }
  // Send remaining Frames
  if(sendPacketBuilder.GetFrameCount() > 0)
  {
    if(RawHID.send(sendPacketBuffer, 10) <= 0) ledPeriod = 100;
  }
  
  // Blink LED
  UpdateLED();
}

void UpdateReceive()
{
  static unsigned char* frameBuffer;

  int receivedBytes = RawHID.recv(recvPacketBuffer, 0);
  if(receivedBytes > 0)
  {
    // Set up PacketBuilder for receiving Frames
    PacketBuilder recvPacketBuilder(recvPacketBuffer);
    for(int frameNum = 0; frameNum < recvPacketBuilder.GetFrameCount(); frameNum++)
    {
      frameBuffer = recvPacketBuilder.GetFrameData(frameNum);
      Frame request(frameBuffer);
      for(int i = 0; i < sizeof(requestHandlers)/sizeof(requestHandlers[0]); i++)
      {
        Frame response;
        byte sendBuffer[Frame::FrameSize];
        if(requestHandlers[i](request, response, sendBuffer) && request.GetType() == Frame::FrameType::Synchron)
        {
          // Set up PacketBuilder for sending Frame
          PacketBuilder sendPacketBuilder(sendPacketBuffer);
          sendPacketBuilder.Flush();
          sendPacketBuilder.AddFrame(response);
          if(RawHID.send(sendPacketBuffer, 10) <= 0) ledPeriod = 100;
        }
      } 
    }
  }
}

void UpdateLED()
{
  static long lastTimestamp = millis();

  if(millis() - lastTimestamp >= ledPeriod)
  {
    static uint8_t state = 0;
    if(ledPeriod > 0) digitalWrite(LED_PIN, state);
    else              digitalWrite(LED_PIN, 1);
    state = state ? 0 : 1;
    lastTimestamp = millis();
  }
}
/*
 * END Main Program Section
 ******************************************************************************/
