#define _GNU_SOURCE

#include <pthread.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include "SocketDriver.h"
#include "debug.h"
#include "universe.h"

//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    FILE GLOBAL TYPES    **************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
#ifdef IS_SERVER
	typedef struct clientNode clientNode;		//available only for server

	struct clientNode {
		int          socketFD;					//file descriptor of this socket
		pthread_t    servicingThread;			//thread servicing this client
		clientNode*  next;						//next client in list
	};

	typedef struct {
		void* forwardedArgs;
		pthread_func_t serviceRoutine;
	} launcherArgs;
#endif

//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    FUNCTION PROTOTYPES    ************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
bool getSocket(int* const fd, int* const sockoptval);
bool socketBind(SocketDriver* const s);

//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    FILE GLOBAL VARIABLES    **********************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
static const int g_domain = AF_INET;
static const int g_type = SOCK_DGRAM;
static const int g_protocol = IPPROTO_UDP;

#ifdef IS_SERVER
	size_t      clientCount = 0;
	clientNode* clientList = NULL;		//available only for server
	pthread_mutex_t clientListLock = PTHREAD_MUTEX_INITIALIZER;
	clientNode* freeClient = NULL;
#endif

//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    GENERAL DRIVER CODE    ************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
SocketDriver getSocketDriver(void) {
	SocketDriver rv = { 0 };			//if field later unspecified, default all zeroes
	rv.fd = -1;							//specify each field with
	rv.sockoptval = -1;
	rv.theirSockInfo = NULL;
	#ifdef IS_SERVER
		serverSocket(&rv);
	#else
		clientSocket(&rv);
	#endif
	return rv;
}

void closeSocketDriver(SocketDriver* const s) {
	ASSERT(s);
	ASSERT(s->fd >= 0);
	close(s->fd);
	if (s->theirSockInfo) {
		LINE_LOG;
		free(s->theirSockInfo);
		s->theirSockInfo = NULL;
	}
}

// @Procedure - opens socket and provides file descriptor
// @return - true if success, false if failed
// @param[fd] - pointer to int where file descriptor of socket
//              should be written, does not write on failure
bool getSocket(int* const fd) {
	ASSERT(fd);							//check for valid params
	ASSERT(sockoptval);
	bool rv = true;						//default true, false by ERROR_IF
	int fdesc = socket(					//check that socket is acquired
		g_domain,
		g_type,
		g_protocol
	);
	ERROR_IF (fdesc < 0);				//error when opening socket fails
	*fd = fdesc;						//set and return fd if socket properly acquired
	doReturn:
		return rv;
}

// @Procedure - opens and binds socket, provides file
//              descriptor and bind result
// @return - true if success, false if failed
// @param[s] - socket data structure to bind results to
bool clientSocket(SocketDriver* const s) {
	ASSERT(s);													//check for valid param
	bool rv = true;												//default true, false by ERROR_IF
	int fd = -1;
	ASSERT(getSocket(&fd));

	memset((char*)&s->theirSockInfo, 0, sizeof(struct sockaddr_in));
	s->theirSockInfo.sin_family = AF_INET;
	s->theirSockInfo.sin_port = htons(PORT);

	ASSERT(inet_aton(SERVER, &s->theirSockInfo.sin_addr) != 0);

	doReturn:
		return rv;
}

// @Procedure - opens and binds socket, provides file
//              descriptor and bind result
// @return - true if success, false if failed
// @param[s] - socket data structure to bind results to
bool serverSocket(SocketDriver* const s) {
	ASSERT(s);													//check for valid param
	bool rv = true;												//default true, false by ERROR_IF
	int fd = -1;
	ASSERT(getSocket(&fd));

	memset((char*)&s->mySockInfo, 0, sizeof(struct sockaddr_in));

	s->mySockInfo.sin_family = AF_INET;
	s->mySockInfo.sin_port = htons(PORT);
	s->mySockInfo.sin_addr.s_addr = htonl(INADDR_ANY);

	ASSERT(bind(fd, (struct sockaddr*)&s->mySockInfo, sizeof(struct sockaddr_in)) != -1);

	doReturn:
		return rv;
}


//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    SERVER DRIVER CODE    *************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
#ifdef IS_SERVER
	void serverListenLoop(SocketDriver* const s, pthread_func_t serviceRoutine) {
		ASSERT(s);
		ASSERT(serviceRoutine);

		CommunicationBuffer_t buffer = { 0 };											//stack buffer for reading client data
		listenLoopRoutineArgs_t dat = { 0 };											//take args as struct

		dat.outData.data = &buffer;														//note buffer location
		dat.outData.dataLength = BUF_SIZE;												//note buffer size
		dat.currentState = WAITING;
		dat.bytesReceived = 0;
		dat.downloadSize = 0;
		dat.nextID = 0;

		while (true) {
			ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen);
		}
	}
#endif