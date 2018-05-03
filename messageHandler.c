#include "messageHandler.h"

//*****************************************************************************************************************
//*****************************************************************************************************************
//***************************************    Function Prototypes    ***********************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
	void downloadWrite(const dataSegment_t* const msg);
	void clearCommBuff(CommunicationBuffer_t* buf);
	void initializeOutgoing(messageType_t msgType);
	void initializeAcknowledge(listenLoopRoutineArgs_t* const dat) ;

	bool validWaitingCondition(listenLoopRoutineArgs_t* const dat);
	bool validDownloadingCondition(listenLoopRoutineArgs_t* const dat);
	bool validFinishedCondition(listenLoopRoutineArgs_t* const dat);

	void processWaiting(listenLoopRoutineArgs_t* const dat);
	void processDownloading(listenLoopRoutineArgs_t* const dat);
	void processFinished(listenLoopRoutineArgs_t* const dat);

	void processMessage(listenLoopRoutineArgs_t* const dat);
	bool messageSensible(listenLoopRoutineArgs_t* const dat);
	void issueFailed(listenLoopRoutineArgs_t* const dat);

	bool handleMessage(listenLoopRoutineArgs_t* const incomingMessage, CommunicationBuffer_t** outgoingHandle);


//*****************************************************************************************************************
//*****************************************************************************************************************
//********************************************    Local Data    ***************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
	CommunicationBuffer_t outgoingBuffer = {{ 0 }};
	FILE* serverWrittenFile = NULL;
	char serverFileCheckSum = 0;


//*****************************************************************************************************************
//*****************************************************************************************************************
//****************************************    Helper Routines   ***************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
	void downloadWrite(const dataSegment_t* const msg) {
		ASSERT(msg);																			//ensure input valid
		ASSERT(serverWrittenFile);																//ensure file handle is good
		const size_t numWritten = BUF_SIZE - sizeof(dataSegment_t);								//take size of buffer
		fwrite(msg->bufferHandle, numWritten, 1, serverWrittenFile);							//write buffer contents to file
		size_t checkSumIterator = 0;
		for (checkSumIterator = 0; checkSumIterator < numWritten; checkSumIterator++) {			//continue calculating file checksum
			serverFileCheckSum = serverFileCheckSum ^ msg->bufferHandle[checkSumIterator];
		}
	}

	void clearCommBuff(CommunicationBuffer_t* buf) {
		ASSERT(buf);													//check valid input buffer pointer
		memset((void*)buf, 0, sizeof(CommunicationBuffer_t));			//zero out contents of buffer
	}

	void initializeOutgoing(messageType_t msgType) {
		ASSERT(LOWER_MSG_INVALID < msgType);							//check valid msgType enum
		ASSERT(msgType < UPPER_MSG_INVALID);
		clearCommBuff(&outgoingBuffer);									//zero out buffer
		outgoingBuffer.asUploadRequest.messageType = msgType;			//write msgType enum
	}

	void initializeAcknowledge(listenLoopRoutineArgs_t* const dat) {
		initializeOutgoing(ACKNOWLEDGE);
		outgoingBuffer.asAcknowledge.messageID = 0;								//Server Ack always ID 0
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
		const baseMessage_t* const msg = (const baseMessage_t*)dat->data;
		const messageType_t        msgType = msg->messageType;
		return (WAITING == currentServerState) && (UPLOAD_REQUEST == msgType) && (dat->nextID == msg->messageID);
	}

	bool validDownloadingCondition(listenLoopRoutineArgs_t* const dat) {
		const serverState_t        currentServerState = dat->currentState;
		const baseMessage_t* const msg = (const baseMessage_t*)dat->data;
		const messageType_t        msgType = msg->messageType;
		return (DOWNLOADING == currentServerState) && (DATA_SEGMENT == msgType) && (dat->nextID == msg->messageID);
	}

	bool validFinishedCondition(listenLoopRoutineArgs_t* const dat) {
		const serverState_t        currentServerState = dat->currentState;
		const baseMessage_t* const msg = (const baseMessage_t*)dat->data;
		const messageType_t        msgType = msg->messageType;
		return (FINISHED == currentServerState) && (END_UPLOAD == msgType) && (dat->nextID == msg->messageID);
	}


//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    State Processor Procedures    *****************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
	void processWaiting(listenLoopRoutineArgs_t* const dat) {
		ASSERT(validWaitingCondition(dat));								//ensure proper state
		const uploadRequest_t* const msg = (uploadRequest_t*)dat->data;	//take message as uploadRequest
		const size_t                 msgSize = msg->fileSize;			//take size of download

		initializeAcknowledge(dat);										//default response
		dat->bytesReceived = 0;											//zero downloaded byte counter
		dat->downloadSize = msgSize;									//initialize filesize
		ASSERT(NULL == serverWrittenFile);								//ensure file NULL
		serverWrittenFile = tmpfile();									//open file
		ASSERT(serverWrittenFile);										//ensure file opened
		dat->currentState = DOWNLOADING;								//set downloading state
	}

	void processDownloading(listenLoopRoutineArgs_t* const dat) {
		ASSERT(validDownloadingCondition(dat));							//ensure proper state
		const dataSegment_t* const msg = (dataSegment_t*)dat->data;		//take message as dataSegment message

		downloadWrite(msg);												//write the contents to the file
		initializeAcknowledge(dat);										//pack the response

		const size_t numWritten = BUF_SIZE - sizeof(dataSegment_t);		//take size of buffer
		dat->bytesReceived = dat->bytesReceived + numWritten;			//increment byte counter

		if (dat->bytesReceived >= dat->downloadSize) {					//check byte counter state
			dat->currentState = FINISHED;
		}
	}

	void processFinished(listenLoopRoutineArgs_t* const dat) {
		ASSERT(validFinishedCondition(dat));							//ensure proper state
		endUpload_t* const msg = (endUpload_t*)dat->data;				//take message as endUpload

		if (msg->checkSum != serverFileCheckSum) {						//ensure proper checksum
			printf("my check = %d, their check = %d\n", (int)serverFileCheckSum, (int)msg->checkSum);
			ASSERT(false);
		}

		size_t lastByteIndex = (BUF_SIZE - sizeof(endUpload_t)) - 1;	//take index of last byte of filename (max supported size - 1)
		msg->fileName[lastByteIndex] = 0;								//write terminal byte with zero, just in case
		ASSERT(serverWrittenFile);										//ensure file handle is good

		FILE* tmpWithName = fopen(msg->fileName, "wb");
		ASSERT(tmpWithName);
		rewind(serverWrittenFile);
		size_t counter = 0;
		for (counter = 0; counter < dat->downloadSize; counter++) {
			fputc( (char)fgetc(serverWrittenFile), tmpWithName);
		}
		fclose(tmpWithName);
		fclose(serverWrittenFile);										//close file handle
		tmpWithName = NULL;
		serverWrittenFile = NULL;										//null out file handle
		dat->currentState = DOWNLOAD_COMPLETE;							//mark state as download complete
		initializeAcknowledge(dat);										//pack the ack response
	}


//*****************************************************************************************************************
//*****************************************************************************************************************
//*************************************    Driver + Overload Procedures    ****************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
	void processMessage(listenLoopRoutineArgs_t* const dat) {
		if (validWaitingCondition(dat)) {
			processWaiting(dat);
		} else if (validDownloadingCondition(dat)) {
			processDownloading(dat);
		} else if (validFinishedCondition(dat)) {
			processFinished(dat);
		}
	}

	bool messageSensible(listenLoopRoutineArgs_t* const dat) {
		const baseMessage_t* const msg = (const baseMessage_t*)dat->data;

		bool rv = false;							//sensible if
		rv = rv || validWaitingCondition(dat);		//wait or
		rv = rv || validDownloadingCondition(dat);	//download or
		rv = rv || validFinishedCondition(dat);		//finished

		if (!rv) {
			const serverState_t        currentServerState = dat->currentState;
			const baseMessage_t* const msg = (const baseMessage_t*)dat->data;
			const messageType_t        msgType = msg->messageType;
			printf("currentServerState = %u\n", currentServerState);
			printf("msgType = %u\n", msgType);
			printf("dat->nextID = %ld\n1", dat->nextID);
			printf("msg->messageID = %ld\n", msg->messageID);
		}

		//ASSERT(rv);
		//ASSERT(dat->nextID == msg->messageID);
		rv = rv && (dat->nextID == msg->messageID);	//and matching ID
		ASSERT(rv);

		return rv;
	}

	void issueFailed(listenLoopRoutineArgs_t* const dat) {
		const baseMessage_t* const msg = (const baseMessage_t*)dat->data;

		initializeOutgoing(MESSAGE_FAILED);										//zero buffer and set to failed message
		messageFailed_t* failedMsg = &outgoingBuffer.asMessageFailed;			//take buffer as messageFailed
		failedMsg->failedMessageID = msg->messageID;							//set ID of failed message
	}

	bool handleMessage(listenLoopRoutineArgs_t* const dat, CommunicationBuffer_t** outgoingHandle) {
		ASSERT(dat);							//check input ptrs valid
		ASSERT(outgoingHandle);

		bool rv = messageSensible(dat);
		if (rv) {								//check message is valid
			processMessage(dat);				//process message
		} else {
			//issueFailed(dat);					//process message failure
		}
		(*outgoingHandle) = &outgoingBuffer;	//send back pointer to outgoing buffer

		return rv;
	}
