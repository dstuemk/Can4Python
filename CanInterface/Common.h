#ifndef __COMMON_H
#define __COMMON_H

#define PACKET_SIZE 254
#define DEBUG_LEVEL 0

// Include Requests
#include "Frames/Requests/AddReceiveHandlerRequest.h"
#include "Frames/Requests/ConfigurationRequest.h"
#include "Frames/Requests/EnableTransreceiverRequest.h"
#include "Frames/Requests/RemoveReceiveHandlerRequest.h"
#include "Frames/Requests/SendFrameRequest.h"

// Include Responses
#include "Frames/Responses/AddReceiveHandlerResponse.h"
#include "Frames/Responses/ConfigurationResponse.h"
#include "Frames/Responses/EnableTransreceiverResponse.h"
#include "Frames/Responses/RemoveReceiveHandlerResponse.h"
#include "Frames/Responses/SendFrameResponse.h"

// Include Events
#include "Frames/Events/FrameReceivedEvent.h"

// Include PacketBuilder
#include "PacketBuilder.h"

#endif
