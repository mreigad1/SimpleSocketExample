#include "universe.h"

typedef enum {
	UPLOAD_REQUEST = 100,
	ACKNOWLEDGE    = 101,
	DATA_SEGMENT   = 102,
	END_UPLOAD     = 103,
	MESSAGE_FAILED = 104
} messageType_t;

typedef enum {
	WAITING      = 200,
	DOWNLOADING  = 201,
	FINISHED     = 202
} serverState_t;

typedef struct {
	messageType_t messageType;
	size_t messageID;
} baseMessage_t;

//UPLOAD_REQUEST
//Client sends
//Server receives
typedef struct {
	baseMessage_t header;
	size_t fileSize;
} uploadRequest_t;

//ACKNOWLEDGE
//Server+Client sends
//Server+Client receives
typedef struct {
	baseMessage_t header;
	unsigned acknowledgedMessageID;
	unsigned nextID;
} acknowledge_t;

//DATA_SEGMENT
//Client sends
//Server receives
typedef struct {
	baseMessage_t header;
	size_t segmentIndex;
	size_t segmentLength;
	char bufferHandle[0];
} dataSegment_t;

//END_UPLOAD
//Client sends
//Server receives
typedef struct {
	baseMessage_t header;
	char checkSum;
	char fileName[0];
} endUpload_t;

//MESSAGE_FAILED
//Server+Client sends
//Server+Client receives
typedef struct {
	baseMessage_t header;
	unsigned failedMessageID;
} messageFailed_t;

typedef union CommunicationBuffer_t {
	uploadRequest_t asUploadRequest;
	acknowledge_t 	asAcknowledge;
	dataSegment_t 	asDataSegment;
	endUpload_t 	asEndUpload;
	messageFailed_t asMessageFailed;
	char 			rawData[BUF_SIZE];
} CommunicationBuffer_t;