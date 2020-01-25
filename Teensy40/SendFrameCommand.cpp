#include "SendFrameCommand.h"

/******************************************************************************
 * BEGIN SendFrame Command Section
 */
bool (*sendCanFrameFunction)(CanFrame&)  = nullptr;

void InitSendCanFrameFunction(bool(*func)(CanFrame&))
{
  sendCanFrameFunction = func;
}

bool SendFrame(Frame& requestFrame, Frame& responseFrame, uint8_t* sendBuffer)
{
  if(Frame::Command::SendFrame != requestFrame.GetCommand()) return false;

  SendFrameRequest request(requestFrame);
  CanFrame canFrame = request.GetCanFrame();

  if(nullptr == sendCanFrameFunction)
  {
    responseFrame = SendFrameResponse(sendBuffer, SendFrameResponse::ResponseType::Failure);
  }
  else if(sendCanFrameFunction(canFrame))
  {
    responseFrame = SendFrameResponse(sendBuffer, SendFrameResponse::ResponseType::Success);
  }
  else 
  {
    responseFrame = SendFrameResponse(sendBuffer, SendFrameResponse::ResponseType::FailureTxQueueFull);
  }

  return true;
}
 /*
 * END SendFrame Command Section
 ******************************************************************************/
