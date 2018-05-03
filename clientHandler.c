#include "clientHandler.h"

//*****************************************************************************************************************
//*****************************************************************************************************************
//***************************************    Function Prototypes    ***********************************************
//*****************************************************************************************************************
//*****************************************************************************************************************

	size_t getFileSize(const char* fileName);
	void clearCommBuff(CommunicationBuffer_t* buf);
	void initializeOutgoing(messageType_t msgType);
	void initializeSegment(listenLoopRoutineArgs_t* const dat);

	void processWaiting(listenLoopRoutineArgs_t* const dat);
	void processDownloading(listenLoopRoutineArgs_t* const dat) ;
	void processLast(listenLoopRoutineArgs_t* const dat);
	void processFinished(listenLoopRoutineArgs_t* const dat);

	void processMessage(listenLoopRoutineArgs_t* const dat);
	bool handleMessage(listenLoopRoutineArgs_t* const dat, CommunicationBuffer_t** outgoingHandle);
	bool downloadRequest(const char* fileName, CommunicationBuffer_t** outgoingHandle);


//*****************************************************************************************************************
//*****************************************************************************************************************
//********************************************    Local Data    ***************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
	CommunicationBuffer_t outgoingBuffer = { 0 };
	FILE* serverWrittenFile = NULL;
	const char* uploadFileName = NULL;
	size_t uploadFileSize = 0;
	char serverFileCheckSum = 0;


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

	void initializeSegment(listenLoopRoutineArgs_t* const dat) {
		ASSERT(serverWrittenFile);													//check file open
		initializeOutgoing(DATA_SEGMENT);

		const size_t numWritten =
			sizeof(CommunicationBuffer_t) - sizeof(dataSegment_t);					//take size of buffer
		fread(
			&outgoingBuffer.asDataSegment.bufferHandle[0],
			numWritten, numWritten, serverWrittenFile
		);																			//read from file
		dat->bytesReceived += numWritten;											//increment num bytes sent
	}


//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    State Processor Procedures    *****************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
	void processWaiting(listenLoopRoutineArgs_t* const dat) {
		ASSERT(dat);
		ASSERT(NULL == serverWrittenFile);								//check file not open

		dat->currentState = DOWNLOADING;								//advance state
		dat->downloadSize = uploadFileSize;								//initialize filesize
		dat->bytesReceived = 0;											//initialize bytes sent

		serverWrittenFile = fopen(uploadFileName, "rb");				//open file for reading binary
		ASSERT(serverWrittenFile);										//check file open

		initializeSegment(dat);											//create a download segment and return
	}

	void processDownloading(listenLoopRoutineArgs_t* const dat) {
		ASSERT(dat);
		if (dat->bytesReceived >= dat->downloadSize) {					//last packet of data completed transmission and acked?
			processLast(dat);
		} else {														//else there are more bytes to send
			initializeSegment(dat);										//create a download segment and return
		}
	}

	void processLast(listenLoopRoutineArgs_t* const dat) {
		ASSERT(dat);
		ASSERT(uploadFileName);		
		initializeOutgoing(END_UPLOAD);									//ensurefilename
		dat->currentState = FINISHED;									//advance state and send filename
		outgoingBuffer.asEndUpload.checkSum = serverFileCheckSum;

		const size_t numWritten =
			sizeof(CommunicationBuffer_t) - sizeof(endUpload_t) - 1;	//maximum length filename supported
		strncpy(
			outgoingBuffer.asEndUpload.fileName,
			uploadFileName,
			numWritten
		);
	}

	void processFinished(listenLoopRoutineArgs_t* const dat) {
		dat->currentState = DOWNLOAD_COMPLETE;							//advance state
		fclose(serverWrittenFile);
		serverWrittenFile = NULL;
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
		outgoingBuffer.asUploadRequest.header.messageID = dat->nextID;	//set ID to next ID
	}

	bool handleMessage(listenLoopRoutineArgs_t* const dat, CommunicationBuffer_t** outgoingHandle) {
		ASSERT(dat);										//check input ptrs valid
		ASSERT(outgoingHandle);
		const acknowledge_t* const msg                   = (acknowledge_t*)dat->data;
		const messageType_t        msgType               = msg->header.messageType;
		messageType_t              acknowledgedMessageID = msg->acknowledgedMessageID;
		bool sensible = (ACKNOWLEDGE == msgType) && (acknowledgedMessageID == dat->nextID);

		if (sensible) {										//was acknowledgement of last message?
			dat->nextID = msg->nextID;						//set ID to ID from ack
			processMessage(dat);							//process the ack / send next message
		} else {
			ASSERT(acknowledgedMessageID == dat->nextID);
			ASSERT(ACKNOWLEDGE == msgType);
		}
		(*outgoingHandle) = &outgoingBuffer;				//send back pointer to outgoing buffer

		return sensible;
	}

	bool downloadRequest(const char* fileName, CommunicationBuffer_t** outgoingHandle) {
		ASSERT(fileName);												//check input ptrs valid
		ASSERT(outgoingHandle);

		if (access(fileName, R_OK)) {									//if file exists
			uploadFileSize = getFileSize(fileName);						//take filesize

			initializeOutgoing(UPLOAD_REQUEST);							//clear buffer and set to upload request

			uploadRequest_t* req = &outgoingBuffer.asUploadRequest;		//take buffer as upload request
			req->header.messageID = 0;									//mark messageID to 0
			req->fileSize = uploadFileSize;								//set file size of target file
			(*outgoingHandle) = &outgoingBuffer;						//send back pointer to outgoing buffer

			uploadFileName = fileName;									//set global filename
			return true;
		}
		return false;
	}