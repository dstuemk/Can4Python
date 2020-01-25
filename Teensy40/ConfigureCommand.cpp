#include "ConfigureCommand.h"

/******************************************************************************
 * BEGIN Configure Command Section
 */
bool (*configureFunction)(ConfigurationRequest&) = nullptr;

void InitConfigureFunction(bool (*func)(ConfigurationRequest&))
{
  configureFunction = func;
}

bool Configure(Frame& requestFrame, Frame& responseFrame, uint8_t* sendBuffer)
{
  if(Frame::Command::Configure != requestFrame.GetCommand()) return false;

  ConfigurationRequest request(requestFrame);
  
  if(nullptr == configureFunction)
  {
    responseFrame = ConfigurationResponse(sendBuffer, ConfigurationResponse::ResponseType::Failure);
  }
  else if(configureFunction(request))
  {
    responseFrame = ConfigurationResponse(sendBuffer, ConfigurationResponse::ResponseType::Success);
  }
  else 
  {
    responseFrame = ConfigurationResponse(sendBuffer, ConfigurationResponse::ResponseType::Failure);
  }

  return true;
}
 /*
 * END Configure Command Section
 ******************************************************************************/
