#include "messageHandler.h"
#include "messageTypes.h"
#include "debug.h"
#include "universe.h"


//*****************************************************************************************************************
//*****************************************************************************************************************
//***************************************    Function Prototypes    ***********************************************
//*****************************************************************************************************************
//*****************************************************************************************************************


//*****************************************************************************************************************
//*****************************************************************************************************************
//********************************************    Local Data    ***************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
	CommunicationBuffer_t outgoingBuffer = { 0 };
	FILE* serverWrittenFile = NULL;
	char serverFileCheckSum = 0;
	const char* const tempFileName = "serverTempDownload.file";


//*****************************************************************************************************************
//*****************************************************************************************************************
//****************************************    Helper Routines   ***************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
	bool downloadWrite(const dataSegment_t* const msg) {
		ASSERT(msg);																			//ensure input valid
		ASSERT(serverWrittenFile);																//ensure file handle is good
		ASSERT(msg->segmentLength <= (BUF_SIZE - sizeof(dataSegment_t)));						//ensure proper packet size has been sent
		fwrite(msg->bufferHandle, msg->segmentLength, 1, serverWrittenFile);					//write buffer contents to file
		size_t checkSumIterator = 0;
		for (checkSumIterator = 0; checkSumIterator < msg->segmentLength; checkSumIterator++) {	//continue calculating file checksum
			serverFileCheckSum = serverFileCheckSum ^ msg->bufferHandle[checkSumIterator];
		}
		return true;
	}

	void clearCommBuff(CommunicationBuffer_t* buf) {
		ASSERT(buf);													//check valid input buffer pointer
		memset((void*)buf, 0, sizeof(CommunicationBuffer_t));			//zero out contents of buffer
	}

	void initializeOutgoing(messageType_t msgType) {
		ASSERT(LOWER_MSG_INVALID < msgType);							//check valid msgType enum
		ASSERT(msgType < UPPER_MSG_INVALID);
		clearCommBuff(&outgoingBuffer);									//zero out buffer
		outgoingBuffer.asUploadRequest.header.messageType = msgType;	//write msgType enum
	}

	void initializeAcknowledge(listenLoopRoutineArgs_t* const dat) {
		initializeOutgoing(ACKNOWLEDGE);
		outgoingBuffer.asAcknowledge.header.messageID = 0;						//Server Ack always ID 0
		outgoingBuffer.asAcknowledge.acknowledgedMessageID = dat->nextID++;		//server acknowledging last ID and incrementing
		outgoingBuffer.asAcknowledge.nextID = dat->nextID;						//packing next ID server is expecting to see
	}


//*****************************************************************************************************************
//*****************************************************************************************************************
//************************************    State Conditional Routines    *******************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
	bool validWaitingCondition(listenLoopRoutineArgs_t* const dat) {
		const serverState_t        currentServerState = dat->currentState;
		const baseMessage_t* const msg = dat->outData.data;
		const messageType_t        msgType = msg->messageType;
		return (WAITING == currentServerState) && (UPLOAD_REQUEST == msgType) && (dat->nextID == msg->messageID);
	}

	bool validDownloadingCondition(listenLoopRoutineArgs_t* const dat) {
		const serverState_t        currentServerState = dat->currentState;
		const baseMessage_t* const msg = dat->outData.data;
		const messageType_t        msgType = msg->messageType;
		return (DOWNLOADING == currentServerState) && (DATA_SEGMENT == msgType) && (dat->nextID == msg->messageID);
	}

	bool validFinishedCondition(listenLoopRoutineArgs_t* const dat) {
		const serverState_t        currentServerState = dat->currentState;
		const baseMessage_t* const msg = dat->outData.data;
		const messageType_t        msgType = msg->messageType;
		return (FINISHED == currentServerState) && (END_UPLOAD == msgType) && (dat->nextID == msg->messageID);
	}



//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    State Processor Procedures    *****************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
	bool processWaiting(listenLoopRoutineArgs_t* const dat) {
		ASSERT(validWaitingCondition(dat));							//ensure proper state
		const uploadRequest_t* const msg = dat->outData.data;		//take message as uploadRequest
		const size_t                 msgSize = msg->fileSize;		//take size of download
		bool rv = false;
		initializeAcknowledge(dat)									//default response
		dat->bytesReceived = 0;										//zero downloaded byte counter
		dat->downloadSize = msgSize;								//initialize filesize
		ASSERT(NULL == serverWrittenFile);							//ensure file NULL
		serverWrittenFile = fopen(tempFileName, "w");				//open file
		ASSERT(serverWrittenFile);									//ensure file opened
		dat->currentState = DOWNLOADING;							//set downloading state
		rv = true;													//return true
		doReturn:
			return rv;
	}

	bool processDownloading(listenLoopRoutineArgs_t* const dat) {
		ASSERT(validDownloadingCondition(dat));							//ensure proper state
		const dataSegment_t* const msg = dat->outData.data;				//take message as dataSegment message
		bool rv = false;
		ASSERT(downloadWrite(msg));										//write the contents to the file
		initializeAcknowledge(dat);										//pack the response
		dat->bytesReceived = dat->bytesReceived + msg->segmentLength;	//increment byte counter

		if (dat->bytesReceived > dat->downloadSize) {					//check byte counter state
			//TODO: Error condition here
		} else if (dat->bytesReceived == dat->downloadSize) {
			dat->currentState = FINISHED;
			rv = true;
		} else {
			rv = true;
		}

		doReturn:
			return rv;
	}

	bool processFinished(listenLoopRoutineArgs_t* const dat) {
		ASSERT(validFinishedCondition(dat));							//ensure proper state
		endUpload_t* const msg = dat->outData.data;						//take message as endUpload

		bool rv = false;
		if (msg->checkSum != serverFileCheckSum) {						//ensure proper checksum
			ASSERT(false);
		}
		size_t lastByteIndex = (BUF_SIZE - sizeof(endUpload_t)) - 1;	//take index of last byte of filename (max supported size - 1)
		msg->fileName[lastByteIndex] = 0;								//write terminal byte with zero, just in case
		ASSERT(serverWrittenFile);										//ensure file handle is good
		fclose(serverWrittenFile);										//close file handle
		serverWrittenFile = NULL;										//null out file handle
		rename(tempFileName, msg->fileName);							//rename file with final name
		dat->currentState = DOWNLOAD_COMPLETE;							//mark state as download complete
		initializeAcknowledge(dat);										//pack the ack response

		doReturn:
			return rv;
	}



//*****************************************************************************************************************
//*****************************************************************************************************************
//*************************************    Driver + Overload Procedures    ****************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
	bool processMessage(listenLoopRoutineArgs_t* const dat) {
		const serverState_t        currentServerState = dat->currentState;
		const baseMessage_t* const msg = dat->outData.data;
		const messageType_t        msgType = msg->messageType;

		bool rv = false;
		if (validWaitingCondition(dat)) {
			rv = processWaiting(dat);
		} else if (validDownloadingCondition(dat)) {
			rv = processDownloading(dat);
		} else if (validFinishedCondition(dat)) {
			rv = processFinished(dat);
		}

		doReturn:
			return rv;
	}

	bool messageSensible(listenLoopRoutineArgs_t* const dat) {
		const serverState_t        currentServerState = dat->currentState;
		const baseMessage_t* const msg = dat->outData.data;
		const messageType_t        msgType = msg->messageType;

		bool rv = false;							//sensible if
		rv = rv || validWaitingCondition(dat);		//wait or
		rv = rv || validDownloadingCondition(dat);	//download or
		rv = rv || validFinishedCondition(dat);		//finished

		ASSERT(rv);
		ASSERT(dat->nextID == msg->messageID);
		rv = rv && (dat->nextID == msg->messageID);	//and matching ID
		ASSERT(rv);

		doReturn:
			return rv;
	}

	void issueFailed(listenLoopRoutineArgs_t* const dat) {
		const serverState_t        currentServerState = dat->currentState;
		const baseMessage_t* const msg = dat->outData.data;
		const messageType_t        msgType = msg->messageType;

		initializeOutgoing(MESSAGE_FAILED);										//zero buffer and set to failed message
		messageFailed_t* failedMsg = &outgoingBuffer.asMessageFailed;			//take buffer as messageFailed
		failedMsg->failedMessageID = msg->messageID;							//set ID of failed message
	}


	bool handleMessage(listenLoopRoutineArgs_t* const incomingMessage, CommunicationBuffer_t** outgoingHandle) {
		ASSERT(incomingMessage);							//check input ptrs valid
		ASSERT(outgoingHandle);

		bool rv = false;
		if (messageSensible(incomingMessage)) {				//check message is valid
			processMessage(incomingMessage);				//process message
			rv = true;
		} else {
			issueFailed(incomingMessage);					//process message failure
		}
		(*outgoingHandle) = &outgoingBuffer;				//send back pointer to outgoing buffer

		doReturn:
			return rv;
	}
