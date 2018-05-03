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
		outgoingBuffer.asUploadRequest.messageType = msgType;			//write msgType enum
	}

	void initializeSegment(listenLoopRoutineArgs_t* const dat) {
		LINE_LOG;
		ASSERT(serverWrittenFile);													//check file open
		LINE_LOG;
		initializeOutgoing(DATA_SEGMENT);
		LINE_LOG;

		const size_t numWritten = BUF_SIZE - sizeof(dataSegment_t);					//take size of buffer
		LINE_LOG;

		fread(
			&outgoingBuffer.asDataSegment.bufferHandle[0],
			numWritten, 1, serverWrittenFile
		);																			//read from file
		LINE_LOG;
		dat->bytesReceived += numWritten;											//increment num bytes sent
		LINE_LOG;

		size_t checkSumIterator = 0;
		LINE_LOG;
		for (checkSumIterator = 0; checkSumIterator < numWritten; checkSumIterator++) {			//continue calculating file checksum
			//LINE_LOG;
			serverFileCheckSum = serverFileCheckSum ^ outgoingBuffer.asDataSegment.bufferHandle[checkSumIterator];
			//LINE_LOG;
		}
		LINE_LOG;
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
		LINE_LOG;
		if (dat->bytesReceived >= dat->downloadSize) {					//last packet of data completed transmission and acked?
			LINE_LOG;
			processLast(dat);
			LINE_LOG;
		} else {														//else there are more bytes to send
			LINE_LOG;
			initializeSegment(dat);										//create a download segment and return
			LINE_LOG;
		}
		LINE_LOG;
	}

	void processLast(listenLoopRoutineArgs_t* const dat) {
		LINE_LOG;
		ASSERT(dat);
		LINE_LOG;
		ASSERT(uploadFileName);	
		LINE_LOG;
		initializeOutgoing(END_UPLOAD);									//ensurefilename
		LINE_LOG;
		dat->currentState = FINISHED;									//advance state and send filename
		LINE_LOG;
		outgoingBuffer.asEndUpload.checkSum = serverFileCheckSum;
		LINE_LOG;

		const size_t numWritten = BUF_SIZE - sizeof(endUpload_t) - 1;	//maximum length filename supported
		LINE_LOG;
		strncpy(
			outgoingBuffer.asEndUpload.fileName,
			"bar.txt",
			//uploadFileName,
			numWritten
		);
		LINE_LOG;
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
		LINE_LOG;
		ASSERT(dat);
		LINE_LOG;
		switch (dat->currentState) {
			case WAITING:
				LINE_LOG;
				processWaiting(dat);
				LINE_LOG;
			break;
			case DOWNLOADING:
				LINE_LOG;
				processDownloading(dat);
				LINE_LOG;
			break;
			case FINISHED:
				LINE_LOG;
				processFinished(dat);
				LINE_LOG;
			break;
			default:
				LINE_LOG;
				ASSERT("Bad state in client" == NULL);
				LINE_LOG;
			break;
		};
		LINE_LOG;
		outgoingBuffer.asUploadRequest.messageID = dat->nextID;	//set ID to next ID
		LINE_LOG;
	}

	bool handleMessage(listenLoopRoutineArgs_t* const dat, CommunicationBuffer_t** outgoingHandle) {
		ASSERT(dat);										//check input ptrs valid
		LINE_LOG;
		ASSERT(outgoingHandle);
		LINE_LOG;
		const acknowledge_t* const msg                   = (acknowledge_t*)dat->data;
		LINE_LOG;
		const messageType_t        msgType               = msg->messageType;
		LINE_LOG;
		messageType_t              acknowledgedMessageID = msg->acknowledgedMessageID;
		LINE_LOG;
		bool sensible = (ACKNOWLEDGE == msgType) && (acknowledgedMessageID == dat->nextID);
		LINE_LOG;

		if (sensible) {										//was acknowledgement of last message?
			LINE_LOG;
			dat->nextID = msg->nextID;						//set ID to ID from ack
			LINE_LOG;
			processMessage(dat);							//process the ack / send next message
			LINE_LOG;
		} else {
			LINE_LOG;
			ASSERT(acknowledgedMessageID == dat->nextID);
			LINE_LOG;
			ASSERT(ACKNOWLEDGE == msgType);
			LINE_LOG;
		}
		LINE_LOG;
		(*outgoingHandle) = &outgoingBuffer;				//send back pointer to outgoing buffer

		LINE_LOG;
		return sensible;
	}

	bool downloadRequest(const char* fileName, CommunicationBuffer_t** outgoingHandle) {
		ASSERT(fileName);												//check input ptrs valid
		ASSERT(outgoingHandle);

		if (0 == access(fileName, F_OK)) {								//if file exists
			uploadFileSize = getFileSize(fileName);						//take filesize

			initializeOutgoing(UPLOAD_REQUEST);							//clear buffer and set to upload request

			uploadRequest_t* req = &outgoingBuffer.asUploadRequest;		//take buffer as upload request
			req->messageID = 0;											//mark messageID to 0
			req->fileSize = uploadFileSize;								//set file size of target file
			(*outgoingHandle) = &outgoingBuffer;						//send back pointer to outgoing buffer

			uploadFileName = fileName;									//set global filename
			return true;
		}
		return false;
	}