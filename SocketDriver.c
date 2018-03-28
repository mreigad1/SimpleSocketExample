#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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
#endif

//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    FUNCTION PROTOTYPES    ************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
bool getSocket(int* const fd, int* const sockoptval);
bool socketBind(SocketDriver* const s);
void serverServiceAccept(const int fd);

//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    FILE GLOBAL VARIABLES    **********************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
static const int g_domain = AF_INET;
static const int g_type = SOCK_STREAM;
static const int g_protocol = 0;
static const int g_bufferSize = 4096;

#ifdef IS_SERVER
	clientNode* clientList = NULL;				//available only for server
#endif

//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    GENERAL DRIVER CODE    ************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
SocketDriver getSocketDriver() {
	SocketDriver rv = { 0 };	//if field later unspecified, default all zeroes
	rv.fd = -1;					//specify each field with
	rv.bindRes = -1;			//explicit default value
	rv.sockoptval = -1;
	rv.socketData = NULL;
	socketBind(&rv);
	return rv;
}

void closeSocketDriver(SocketDriver* const s) {
	ASSERT(s);
	ASSERT(s->fd >= 0);
	close(s->fd);
}

// @Procedure - opens socket and provides file descriptor
// @return - true if success, false if failed
// @param[fd] - pointer to int where file descriptor of socket
//              should be written, does not write on failure
// @param[sockoptval] - pointer to sockOptVal buffer when
//                      invoking upon server
bool getSocket(int* const fd, int* const sockoptval) {
	ASSERT(fd);							//check for valid params
	ASSERT(sockoptval);
	bool rv = true;						//default true, false by ERROR_IF
	int fdesc = socket(					//check that socket is acquired
		g_domain,
		g_type,
		g_protocol
	);
	ERROR_IF (fdesc < 0);				//error when opening socket fails
	#ifdef IS_SERVER					//following section only when server
		*sockoptval = 1;				//parameter for setsockopt
		int serverOptRes = setsockopt(
			fdesc,
			SOL_SOCKET,
			SO_REUSEADDR,
			sockoptval,
			sizeof(*sockoptval)
		);
		ERROR_IF (serverOptRes != 0);	//error is server opt
	#endif
	*fd = fdesc;						//set and return fd if socket properly acquired
	doReturn:
		if (false == rv) {				//on error flag sockoptval negative
			*sockoptval = -1;
		}
		return rv;
}

// @Procedure - opens and binds socket, provides file
//              descriptor and bind result
// @return - true if success, false if failed
// @param[s] - socket data structure to bind results to
bool socketBind(SocketDriver* const s) {
	ASSERT(s);													//check for valid param
	bool rv = true;												//default true, false by ERROR_IF
	int socketResult = -1; 										//default socket/stream returns invalid
	int bindResult = -1;
	bool getSocketSuccess = getSocket(							//get socket file descriptor and set opt val
		&socketResult,
		&s->sockoptval
	);
	ERROR_IF (false == getSocketSuccess);						//error if socket not acquired
	struct sockaddr_in* sockInfo = malloc(sizeof(sockaddr_in));	//allocate sockaddr info and assign data to instance
	ASSERT(NULL != sockInfo);									//check that buffer was successfully acquired
	sockInfo->sin_family = g_domain;							//assign values
	sockInfo->sin_addr.s_addr = htonl(INADDR_ANY);
	#ifdef IS_SERVER
		sockInfo->sin_port = SERVER_PORT_NO;					//if server then use server port num
	#else
		sockInfo->sin_port = htons(0);							//else use OS assigned port
	#endif

	bindResult = bind(											//attempt socket binding
		socketResult,
		sockInfo,
		sizeof(sockInfo)
	);	
	ERROR_IF (bindResult < 0);									//error if socket fails binding	
	(*s) = { socketResult, bindResult, (sockaddr*)sockInfo };	//reaching this point means success,
																//return intermediate values to input struct

	doReturn:
		
		if (false == rv) {										//if error
			if (socketResult >= 0) { close(socketResult); }		//if socket opened but binding failed then close socket
			if (NULL != sockInfo)  { free(sockInfo); }			//clean up allocated struct on error
		}
		return rv;
}

//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    CLIENT DRIVER CODE    *************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
#ifndef IS_SERVER
	bool clientConnect(SocketDriver* const s, int ipComponents[NUM_IP_COMPS]) {
		ASSERT(s);														//verify inputs valid
		ASSERT(ipComponents);
		bool rv = true;													//assume success
		const int numIPComponents = NUM_IP_COMPS;						//number of components in IPv4
		char* sin_addr = &(((sockaddr_in*)s->socketData)->sin_addr);	//take address of buffer for memcpy
		unsigned char ipCastdown[numIPComponents] = { 					//convenient casting array for copying ip address
			ipComponents[0],
			ipComponents[1],
			ipComponents[2],
			ipComponents[3]
		};
		memcpy(sin_addr, ipCastdown, numIPComponents);					//copy ip address into struct
		if (0 != connect(s->fd, s->socketData, sizeof(sockaddr_in))) {	
			memset(sin_addr, 0, numIPComponents);						//on failure to connect, wipe buffer
			rv = false;													//flag ret val to failure
		}
		return rv;
	}
#endif

//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    SERVER DRIVER CODE    *************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
#ifdef IS_SERVER
	bool serverSetListen(SocketDriver* const s) {
		ASSERT(s);														//verify input valid
		const int backlog = 5;											//number of backlogged connections permitted on socket
		bool rv = true;													//default success
		ERROR_IF(listen(s->fd, backlog) != 0);							//error on failed listen
		doReturn:
			return rv;
	}

	void serverListenLoop(SocketDriver* const s) {
		while (true) {
			int requestDescriptor = -1;									//file descriptor of next issued accept
			do {
				requestDescriptor = accept(								//attempt to accept next
					svc,
					(struct sockaddr *)&client_addr,
					&alen
				);
				if (requestDescriptor < 0) {							//broke accept for error 
					const int error = errno;							//capture error number
					static const int acceptableErrors[] = {
						ECHILD,
						ERESTART,
						EINTR
					};													//list of errors tolerated
					
					const int arrSize =
						sizeof(acceptableErrors) / sizeof(int);
					int counter = 0;
					bool isAcceptableError = false;
					for (; counter < arrSize; counter++) {				//if tolerated error then flag appropriately
						if (acceptableErrors[counter] == error) {
							isAcceptableError = true;
						}
					}
					if (false == isAcceptableError) {					//if not tolerated error then display error and die
						fprintf(
							cerr,
							"accept failed for error number: %d",
							error
						);
						exit(1);
					}
				}
			} while (requestDescriptor < 0);
				serverServiceAccept(requestDescriptor);					//new valid accept received, service it
		}
	}

	// @Procedure - receives file descriptor returned from accept()
	//              services and manages newly accepted socket
	// @return - None
	// @param[fd] - file descriptor of new request
	void serverServiceAccept(const int fd) {
		clientNode* newNode = (clientNode)malloc(sizeof(clientNode));
	}
#endif