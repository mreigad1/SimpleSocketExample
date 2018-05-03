#include "SocketDriver.h"

#ifdef IS_SERVER
	#include "messageHandler.h"
#else
	#include "clientHandler.h"
#endif


//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    FUNCTION PROTOTYPES    ************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
	bool getSocket(int* const fd);
	bool acquireSocket(SocketDriver* const s);

//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    FILE GLOBAL VARIABLES    **********************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
	static const int g_domain = AF_INET;
	static const int g_type = SOCK_DGRAM;
	static const int g_protocol = IPPROTO_UDP;

	CommunicationBuffer_t incomingBuffer = {{ 0 }};

//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    GENERAL DRIVER CODE    ************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
	SocketDriver getSocketDriver(void) {
		SocketDriver rv = { 0 };			//if field later unspecified, default all zeroes
		rv.fd = -1;							//specify each field with
		rv.sockoptval = -1;
		acquireSocket(&rv);					//implementation varies as client/server
		ASSERT(rv.fd >= 0);
		return rv;
	}

	void closeSocketDriver(SocketDriver* const s) {
		ASSERT(s);
		ASSERT(s->fd >= 0);
		close(s->fd);
		s->fd = -1;
	}

	// @Procedure - opens socket and provides file descriptor
	// @return - true if success, false if failed
	// @param[fd] - pointer to int where file descriptor of socket
	//              should be written, does not write on failure
	bool getSocket(int* const fd) {
		ASSERT(fd);							//check for valid params
		bool rv = true;						//default true, false by ERROR_IF
		int fdesc = socket(					//check that socket is acquired
			g_domain,
			g_type,
			g_protocol
		);

		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 100 * 1000;
		if (setsockopt(rcv_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
			perror("Socket set timeout failed");
		}

		ASSERT(fdesc >= 0);					//error when opening socket fails
		*fd = fdesc;						//set and return fd if socket properly acquired

		return rv;
	}

	// @Procedure - opens and binds socket, provides file
	//              descriptor and bind result
	// @return - true if success, false if failed
	// @param[s] - socket data structure to bind results to
	bool acquireSocket(SocketDriver* const s) {
		ASSERT(s);													//check for valid param
		bool rv = true;												//default true, false by ERROR_IF
		int fd = -1;
		ASSERT(getSocket(&fd));

		s->fd = fd;

		#ifdef IS_SERVER
			memset((char*)&s->mySockInfo, 0, sizeof(struct sockaddr_in));
			s->mySockInfo.sin_family = g_domain;
			s->mySockInfo.sin_port = htons(SERVER_PORT_NO);
			s->mySockInfo.sin_addr.s_addr = htonl(INADDR_ANY);
			ASSERT(bind(fd, (struct sockaddr*)&s->mySockInfo, sizeof(struct sockaddr_in)) == 0);
		#else
			memset((char*)&s->theirSockInfo, 0, sizeof(struct sockaddr_in));
			s->theirSockInfo.sin_family = g_domain;
			s->theirSockInfo.sin_port = htons(SERVER_PORT_NO);
			ASSERT(inet_aton(SERVER_IP, &s->theirSockInfo.sin_addr) != 0);
		#endif

		return rv;
	}

//*****************************************************************************************************************
//*****************************************************************************************************************
//*****************************************    DRIVER CODE    *****************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
	void driverLoop(SocketDriver* const s) {
		ASSERT(s);

		ASSERT(sizeof(CommunicationBuffer_t) >= BUF_SIZE);

		unsigned int slen = sizeof(struct sockaddr_in);

		listenLoopRoutineArgs_t dat = { 0, 0, 0, WAITING, &incomingBuffer };
		CommunicationBuffer_t* outgoingHandle = NULL;

		//if client
		#ifndef IS_SERVER
			ASSERT(downloadRequest("foo.txt", &outgoingHandle));	//get kickoff message
			LINE_LOG;
			ASSERT(outgoingHandle);
			LINE_LOG;
			ssize_t foo = sendto(s->fd, (void*)outgoingHandle, BUF_SIZE, 0, (struct sockaddr*) &s->theirSockInfo, slen);
			LINE_LOG;
			ASSERT(BUF_SIZE == foo);		//send kickoff message
			LINE_LOG;
		#endif

		while (DOWNLOAD_COMPLETE != dat.currentState) {
			LINE_LOG;
			ssize_t var = recvfrom(s->fd, (void*)&incomingBuffer, BUF_SIZE, 0, (struct sockaddr*)&s->theirSockInfo, &slen);
			LINE_LOG;
			ASSERT(BUF_SIZE == var);	//receive message
			LINE_LOG;
			ASSERT(handleMessage(&dat, &outgoingHandle));																			//handle message, by pack outgoing message
			LINE_LOG;
			ASSERT(outgoingHandle);																									//ensure handle to outgoing message
			LINE_LOG;
			ASSERT(BUF_SIZE == sendto(s->fd, (void*)outgoingHandle, BUF_SIZE, 0, (struct sockaddr*) &s->theirSockInfo, slen));		//send outgoing message
			LINE_LOG;
			outgoingHandle = NULL;																									//nullify outgoing message
			LINE_LOG;
		}

		#ifndef IS_SERVER
			printf("Client Download Completed\n");
		#else
			printf("Server Download Completed\n");
		#endif
	}
