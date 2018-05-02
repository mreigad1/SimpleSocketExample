#include "messageHandler.h"
#include "messageTypes.h"
#include "debug.h"
#include "universe.h"

//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    Valid Processing States    ********************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
bool validWaitingCondition(listenLoopRoutineArgs_t* const dat) {
	const serverState_t        currentServerState = dat->currentState;
	const baseMessage_t* const msg = dat->outData.data;
	const messageType_t        msgType = msg->messageType;
	return (WAITING == currentServerState) && (UPLOAD_REQUEST == msgType);
}

bool validDownloadingCondition(listenLoopRoutineArgs_t* const dat) {
	const serverState_t        currentServerState = dat->currentState;
	const baseMessage_t* const msg = dat->outData.data;
	const messageType_t        msgType = msg->messageType;
	return (DOWNLOADING == currentServerState) && (DATA_SEGMENT == msgType);
}

bool validFinishedCondition(listenLoopRoutineArgs_t* const dat) {
	const serverState_t        currentServerState = dat->currentState;
	const baseMessage_t* const msg = dat->outData.data;
	const messageType_t        msgType = msg->messageType;
	return (FINISHED == currentServerState) && (END_UPLOAD == msgType);
}

//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    State Processor Procedures    *****************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
bool processWaiting(listenLoopRoutineArgs_t* const dat) {
	ASSERT(validWaitingCondition(dat));
	const acknowledge_t* const msg = dat->outData.data;
	const size_t               msgSize = msg->fileSize;

	bool rv = false;
	CommunicationBuffer_t response = { 0 };
	response.asAcknowledge.header.messageType = ACKNOWLEDGE;
	response.asAcknowledge.header.messageID = 0;
	response.asAcknowledge.acknowledgedMessageID = 0;
	response.asAcknowledge.nextID = 1;

	ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);

	dat->downloadSize = msgSize;
	rv = true;

	doReturn:
		return rv;
}

bool processDownloading(listenLoopRoutineArgs_t* const dat) {
	ASSERT(validDownloadingCondition(dat));
	const serverState_t        currentServerState = dat->currentState;
	const baseMessage_t* const msg = dat->outData.data;
	const messageType_t        msgType = msg->messageType;

	bool rv = false;

	doReturn:
		return rv;
}

bool processFinished(listenLoopRoutineArgs_t* const dat) {
	ASSERT(validFinishedCondition(dat));
	const serverState_t        currentServerState = dat->currentState;
	const baseMessage_t* const msg = dat->outData.data;
	const messageType_t        msgType = msg->messageType;

	bool rv = false;

	doReturn:
		return rv;
}

//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    State Processor Procedures    *****************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
bool messageSensible(listenLoopRoutineArgs_t* const dat) {
	const serverState_t        currentServerState = dat->currentState;
	const baseMessage_t* const msg = dat->outData.data;
	const messageType_t        msgType = msg->messageType;

	bool rv = false;
	rv = rv || validWaitingCondition(dat);
	rv = rv || validDownloadingCondition(dat);
	rv = rv || validFinishedCondition(dat);

	doReturn:
		return rv;
}

void issueFailed(listenLoopRoutineArgs_t* const dat) {

}

bool processMessage(listenLoopRoutineArgs_t* const dat) {
	const serverState_t        currentServerState = dat->currentState;
	const baseMessage_t* const msg = dat->outData.data;
	const messageType_t        msgType = msg->messageType;

	bool rv = false;

	if (validWaitingCondition(dat)) {

	} else if (validDownloadingCondition(dat)) {

	} else if (validFinishedCondition(dat)) {

	}

	doReturn:
		return rv;
}

bool receiveAndDone(listenLoopRoutineArgs_t* const dat) {
	bool rv = false;

	if (messageSensible(dat)) {

	} else {
		issueFailed(dat);
	}

	doReturn:
		return rv;
}