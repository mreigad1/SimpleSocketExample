#ifndef MESSAGETYPES_H
#define MESSAGETYPES_H

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpedantic"

		#ifndef UNIVERSE_H
			#include "universe.h"
		#endif

		typedef enum {
			LOWER_MSG_INVALID  =  99,
			UPLOAD_REQUEST     = 100,
			ACKNOWLEDGE        = 101,
			DATA_SEGMENT       = 102,
			END_UPLOAD         = 103,
			MESSAGE_FAILED     = 104,
			UPPER_MSG_INVALID  = 105
		} messageType_t;

		typedef enum {
			WAITING           = 200,
			DOWNLOADING       = 201,
			FINISHED          = 202,
			DOWNLOAD_COMPLETE = 203
		} serverState_t;

		typedef struct {
			messageType_t messageType;
			size_t messageID;
		} baseMessage_t;

		//UPLOAD_REQUEST
		//Client sends
		//Server receives
		typedef struct {
			messageType_t messageType;
			size_t messageID;
			size_t fileSize;
		} uploadRequest_t;

		//ACKNOWLEDGE
		//Server+Client sends
		//Server+Client receives
		typedef struct {
			messageType_t messageType;
			size_t messageID;
			unsigned acknowledgedMessageID;
			unsigned nextID;
		} acknowledge_t;

		//DATA_SEGMENT
		//Client sends
		//Server receives
		typedef struct {
			messageType_t messageType;
			size_t messageID;
			char bufferHandle[0];
		} dataSegment_t;

		//END_UPLOAD
		//Client sends
		//Server receives
		typedef struct {
			messageType_t messageType;
			size_t messageID;
			char checkSum;
			char fileName[0];
		} endUpload_t;

		//MESSAGE_FAILED
		//Server+Client sends
		//Server+Client receives
		typedef struct {
			messageType_t messageType;
			size_t messageID;
			unsigned failedMessageID;
		} messageFailed_t;

		typedef union {
			uploadRequest_t asUploadRequest;
			acknowledge_t 	asAcknowledge;
			dataSegment_t 	asDataSegment;
			endUpload_t 	asEndUpload;
			messageFailed_t asMessageFailed;
			char 			rawData[BUF_SIZE];
		} CommunicationBuffer_t;

	#pragma GCC diagnostic pop

#endif