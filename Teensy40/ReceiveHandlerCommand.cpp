#include "ReceiveHandlerCommand.h"

/******************************************************************************
 * BEGIN ReceiveHandler Command Section
 */ 
struct CanMessageHandler
{
  CanMessageHandler* Before;
  CanMessageHandler* After;
  uint16_t    HandlerId;
  uint8_t     Active;
  uint8_t     FilterType;
  int8_t      ExtendedIdentifier;
  uint32_t    reserved[0];
};

const int canMessageHandlerSize = sizeof(CanMessageHandler) + sizeof(uint32_t)*2;
const int maxCanMessageHandlers = 64;
uint8_t   canMessageHandlersRaw[maxCanMessageHandlers * canMessageHandlerSize];
CanMessageHandler* canMessageHandlerListFree;
CanMessageHandler* canMessageHandlerListUsed;

struct CanMessageHandler* GetCanMessageHandler(int num)
{
  uint8_t* p = &canMessageHandlersRaw[num * canMessageHandlerSize];
  return (CanMessageHandler*)p;
}

void InitCanMessageHandlers()
{
  for(int i = 0; i < maxCanMessageHandlers; i++)
  {
    CanMessageHandler* p = GetCanMessageHandler(i);
    p->HandlerId = i;
    p->Active = 0;
    if(i != 0) p->Before = GetCanMessageHandler(i-1);
    else p->Before = nullptr;
    if(i+1 < maxCanMessageHandlers) p->After = GetCanMessageHandler(i+1);
    else p->After = nullptr;
  }

  canMessageHandlerListFree = GetCanMessageHandler(0);
  canMessageHandlerListUsed = nullptr;
}

CanMessageHandler* NewUsedCanMessageHandler()
{
  // All Message Handlers already in use
  if(nullptr == canMessageHandlerListFree) return nullptr;

  CanMessageHandler* oldFirst = canMessageHandlerListFree;  
  canMessageHandlerListFree = canMessageHandlerListFree->After;
  
  if(nullptr != canMessageHandlerListFree) canMessageHandlerListFree->Before = nullptr;

  oldFirst->After = nullptr;
  oldFirst->Before = nullptr;

  if(nullptr == canMessageHandlerListUsed)
  {
    canMessageHandlerListUsed = oldFirst;
  }
  else 
  {
    canMessageHandlerListUsed->Before = oldFirst;
    oldFirst->After = canMessageHandlerListUsed;
    canMessageHandlerListUsed = oldFirst;
  }

  canMessageHandlerListUsed->Active = 1;
  return canMessageHandlerListUsed;
}

bool FreeUsedCanMessageHandler(uint16_t handlerId)
{
  if(handlerId >= maxCanMessageHandlers) return false;

  CanMessageHandler* messageHandler = GetCanMessageHandler(handlerId);

  if(!messageHandler->Active) return false;

  if(nullptr != messageHandler->Before)
  {
    messageHandler->Before->After = messageHandler->After;
  }

  if(nullptr != messageHandler->After)
  {
    messageHandler->After->Before = messageHandler->Before;
  }

  messageHandler->After = nullptr;
  messageHandler->Before = nullptr;

  if(nullptr == canMessageHandlerListFree)
  {
    canMessageHandlerListFree = messageHandler;
  }
  else 
  {
    canMessageHandlerListFree->Before = messageHandler;
    messageHandler->After = canMessageHandlerListFree;
    canMessageHandlerListFree = messageHandler;
  }

  canMessageHandlerListFree->Active = 0;
  return true;
}

uint32_t GetSingleIdentifier(CanMessageHandler* messageHandler)
{
  return messageHandler->reserved[0];
}

uint32_t GetLowerIdentifier(CanMessageHandler* messageHandler)
{
  return messageHandler->reserved[0];
}

uint32_t GetHigherIdentifier(CanMessageHandler* messageHandler)
{
  return messageHandler->reserved[1];
}

void SetSingleIdentifier(CanMessageHandler* messageHandler, uint32_t identifier)
{
  messageHandler->reserved[0] = identifier;
}

void SetLowerIdentifier(CanMessageHandler* messageHandler, uint32_t identifier)
{
  messageHandler->reserved[0] = identifier;
}

void SetHigherIdentifier(CanMessageHandler* messageHandler, uint32_t identifier)
{
  messageHandler->reserved[1] = identifier;
}

bool CheckCanMessage(CanMessageHandler* messageHandler, CanFrame& canFrame)
{
  if(!messageHandler->Active) return false; 

  switch(messageHandler->FilterType)
  {
    case AddReceiveHandlerRequest::FilterType::All: 
    return true;

    case AddReceiveHandlerRequest::FilterType::Single: 
    return GetSingleIdentifier(messageHandler) == canFrame.GetIdentifier() 
        && (messageHandler->ExtendedIdentifier < 0 || (messageHandler->ExtendedIdentifier == canFrame.ExtendedIdentifier()));

    case AddReceiveHandlerRequest::FilterType::Range: 
    return GetLowerIdentifier(messageHandler) <= canFrame.GetIdentifier()
        && GetHigherIdentifier(messageHandler) >= canFrame.GetIdentifier()
        && (messageHandler->ExtendedIdentifier < 0 || (messageHandler->ExtendedIdentifier == canFrame.ExtendedIdentifier()));
    
    default: 
    return false;
  }
}

bool ProcessCanMessage(CanFrame& canFrame, Frame& eventFrame, uint8_t* sendBuffer)
{
  for(CanMessageHandler* messageHandler = canMessageHandlerListUsed; messageHandler != nullptr; messageHandler = messageHandler->After)
  {
    if(CheckCanMessage(messageHandler, canFrame))
    {
      eventFrame = FrameReceivedEvent(sendBuffer, canFrame);
      return true;
    }
  }
  return false;
}

bool RemoveReceiveHandler(Frame& requestFrame, Frame& responseFrame, uint8_t* sendBuffer)
{
  if(Frame::Command::RemoveReceiveHandler != requestFrame.GetCommand()) return false;

  RemoveReceiveHandlerRequest request(requestFrame);

  if(request.GetHandlerId() == -1)
  {
    // Remove all
    InitCanMessageHandlers();
    responseFrame = RemoveReceiveHandlerResponse(sendBuffer, RemoveReceiveHandlerResponse::ResponseType::Success);
  }
  else 
  {
    if(FreeUsedCanMessageHandler(request.GetHandlerId()))
    {
      responseFrame = RemoveReceiveHandlerResponse(sendBuffer, RemoveReceiveHandlerResponse::ResponseType::Success);
    }
    else 
    {
      responseFrame = RemoveReceiveHandlerResponse(sendBuffer, RemoveReceiveHandlerResponse::ResponseType::FailureWrongHandlerId);
    }
  }
  return true;
}

bool AddReceiveHandler(Frame& requestFrame, Frame& responseFrame, uint8_t* sendBuffer)
{
  if(Frame::Command::AddReceiveHandler != requestFrame.GetCommand()) return false;

  AddReceiveHandlerRequest request(requestFrame);
  CanMessageHandler* messageHandler = NewUsedCanMessageHandler();
  
  if(nullptr == messageHandler)
  {
    responseFrame = AddReceiveHandlerResponse(sendBuffer, AddReceiveHandlerResponse::ResponseType::FailureTooManyReceiveHandlers);
    return true;
  }

  messageHandler->FilterType = request.GetFilterType();

  switch(request.GetFilterType())
  {
    case AddReceiveHandlerRequest::FilterType::All:
    responseFrame = AddReceiveHandlerResponse(sendBuffer, AddReceiveHandlerResponse::ResponseType::Success, messageHandler->HandlerId);
    break;
    
    case AddReceiveHandlerRequest::FilterType::Single:
    responseFrame = AddReceiveHandlerResponse(sendBuffer, AddReceiveHandlerResponse::ResponseType::Success, messageHandler->HandlerId);
    messageHandler->ExtendedIdentifier = request.UseExtendedIdentifiers();
    SetSingleIdentifier(messageHandler, request.GetCanIdentifier());
    break;

    case AddReceiveHandlerRequest::FilterType::Range:
    responseFrame = AddReceiveHandlerResponse(sendBuffer, AddReceiveHandlerResponse::ResponseType::Success, messageHandler->HandlerId);
    messageHandler->ExtendedIdentifier = request.UseExtendedIdentifiers();
    SetLowerIdentifier(messageHandler, request.GetCanIdentifierLow());
    SetHigherIdentifier(messageHandler, request.GetCanIdentifierHigh());
    break;
  }
  
  return true;
}
/*
 * END ReceiveHandler Command Section
 ******************************************************************************/
