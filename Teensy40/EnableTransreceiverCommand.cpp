#include "EnableTransreceiverCommand.h"

/******************************************************************************
 * BEGIN Enable Transreceiver Section
 */
bool (*enableTransreceiverFunction)(bool) = nullptr;

void InitEnableTransreceiverFunction(bool (*func)(bool))
{
  enableTransreceiverFunction = func;
}

bool EnableTransreceiver(Frame& requestFrame, Frame& responseFrame, uint8_t* sendBuffer)
{
  if(Frame::Command::EnableTransreceiver != requestFrame.GetCommand()) return false;

  EnableTransreceiverRequest request(requestFrame);

  if(nullptr == enableTransreceiverFunction)
  {
    responseFrame = EnableTransreceiverResponse(sendBuffer, EnableTransreceiverResponse::ResponseType::Failure);
  }
  else if(enableTransreceiverFunction(request.GetEnableState() == EnableTransreceiverRequest::EnableState::On))
  {
    responseFrame = EnableTransreceiverResponse(sendBuffer, EnableTransreceiverResponse::ResponseType::Success);
  }
  else 
  {
    responseFrame = EnableTransreceiverResponse(sendBuffer, EnableTransreceiverResponse::ResponseType::Failure);
  }

  return true;
}
 /*
 * END Enable Transreceiver Section
 ******************************************************************************/
