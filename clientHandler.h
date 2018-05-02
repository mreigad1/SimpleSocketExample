#include "SocketDriver.h"

bool downloadRequest(const char* fileName, CommunicationBuffer_t** outgoingHandle);

bool handleMessage(listenLoopRoutineArgs_t* const incomingMessage, CommunicationBuffer_t** outgoingHandle);