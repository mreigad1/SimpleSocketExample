#include "clientHandler.h"
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
	const char* uploadFileName = NULL;
	char serverFileCheckSum = 0;

	size_t bytesSent = 0;
	size_t uploadSize = 0;


//*****************************************************************************************************************
//*****************************************************************************************************************
//****************************************    Helper Routines   ***************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
	size_t getFileSize(const char* fileName) {
		ASSERT(fileName);
		FILE* fp = fopen(fileName, "r");
		ASSERT(fp);
		fseek(fp, 0L, SEEK_END);
		size_t sz = ftell(fp);
		rewind(fp);
		fclose(fp);
		fp = NULL;
		return sz;
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

	void initializeSegment(listenLoopRoutineArgs_t* const dat) {
		initializeOutgoing(DATA_SEGMENT);
		size_t numWritten = 
		outgoingBuffer.asDataSegment.segmentLength
	}


//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    State Processor Procedures    *****************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
	void processWaiting(listenLoopRoutineArgs_t* const dat) {
		dat->currentState = DOWNLOADING;								//advance state
		ASSERT(NULL == serverWrittenFile);								//check file not open
		serverWrittenFile = fopen(uploadFileName, "r");					//open file for reading
		ASSERT(serverWrittenFile);										//check file open
		initializeSegment(dat);											//create a download segment and return
	}

	void processDownloading(listenLoopRoutineArgs_t* const dat) {

	}

	void processFinished(listenLoopRoutineArgs_t* const dat) {
		dat->currentState = DOWNLOAD_COMPLETE;							//advance state


	}


//*****************************************************************************************************************
//*****************************************************************************************************************
//*************************************    Driver + Overload Procedures    ****************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
	void processMessage(listenLoopRoutineArgs_t* const dat) {
		ASSERT(dat);
		switch (dat->currentState) {
			case WAITING:
				processWaiting(dat);
			break;
			case DOWNLOADING:
				processDownloading(dat);
			break;
			case FINISHED:
				processFinished(dat);
			break;
			default:
				ASSERT("Bad state in client" == NULL);
			break;
		};
	}

	bool handleMessage(listenLoopRoutineArgs_t* const incomingMessage, CommunicationBuffer_t** outgoingHandle) {
		ASSERT(incomingMessage);							//check input ptrs valid
		ASSERT(outgoingHandle);
		const acknowledge_t* const msg                   = dat->outData.data;
		const messageType_t        msgType               = msg->header.messageType;
		messageType_t              acknowledgedMessageID = msg->acknowledgedMessageID;
		bool sensible = (ACKNOWLEDGE == msgType) && (acknowledgedMessageID == dat->nextID);

		if (sensible) {										//was acknowledgement of last message?
			dat->nextID = msg->nextID;						//set ID to ID from ack
			processMessage(incomingMessage);				//process the ack / send next message
		}
		(*outgoingHandle) = &outgoingBuffer;				//send back pointer to outgoing buffer

		return sensible;
	}

	bool downloadRequest(const char* fileName, CommunicationBuffer_t** outgoingHandle) {
		ASSERT(fileName);							//check input ptrs valid
		ASSERT(outgoingHandle);

		if (access(fileName, "r")) {									//if file exists
			initializeOutgoing(UPLOAD_REQUEST);							//clear buffer and set to upload request
			uploadRequest_t* req = &outgoingBuffer.asUploadRequest;		//take buffer as upload request
			req->header.messageID = 0;									//mark messageID to 0
			req->fileSize = getFileSize(fileName);						//set file size of target file
			uploadFileName = fileName;									//set global filename
			(*outgoingHandle) = req;									//send back pointer to outgoing buffer
			return true;
		}
		return false;
	}