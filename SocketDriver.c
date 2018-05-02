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

	CommunicationBuffer_t incomingBuffer = { 0 };

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
	void serverListenLoop(SocketDriver* const s) {
		ASSERT(s);

		int slen = sizeof(struct sockaddr_in);
		CommunicationBuffer_t* outgoingHandle = NULL;
		listenLoopRoutineArgs_t dat = { 0 };											//take args as struct
		dat.outData.data = &incomingBuffer;												//note buffer location
		dat.outData.dataLength = BUF_SIZE;												//note buffer size
		dat.currentState = WAITING;
		dat.bytesReceived = 0;
		dat.downloadSize = 0;
		dat.nextID = 0;

		while (DOWNLOAD_COMPLETE != dat.currentState) {
			ssize_t numReceived = recvfrom(dat.fd, dat.outData.data, dat.outData.dataLength, 0, (struct sockaddr*) &s->mySockInfo, &slen);
			ASSERT(BUF_SIZE == numReceived);
			ASSERT(handleMessage(&dat, &outgoingHandle));
			sendto(dat.fd, dat.outData.data, dat.outData.dataLength, 0, (struct sockaddr*) &s->mySockInfo, slen);
			outgoingHandle = NULL;
		}
	}
#endif

//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    CLIENT DRIVER CODE    *************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
#ifndef IS_SERVER
	void clientSendLoop(SocketDriver* const s) {

	}
#endif