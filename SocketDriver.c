#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "SocketDriver.h"
#include "debug.h"

static const int g_domain = AF_INET;
static const int g_type = SOCK_STREAM;
static const int g_protocol = 0;
static const int g_bufferSize = 4096;

bool getSocket(int* const fd);
bool socketBind(SocketDriver* const socketDat);

// @Procedure - opens socket and provides file descriptor
// @return - true if success, false if failed
// @param[fd] - pointer to int where file descriptor of socket
//              should be written, does not write on failure
bool getSocket(int* const fd) {
	ASSERT(fd);						//check for valid params
	bool rv = true;					//default true, false by ERROR_IF
	int fdesc = socket(				//check that socket is acquired
		g_domain,
		g_type,
		g_protocol
	);
	ERROR_IF (fdesc < 0);
	*fd = fdesc;					//set and return fd if socket properly acquired
	doReturn:
		return rv;
}

// @Procedure - opens and binds socket, provides file
//              descriptor and bind result
// @return - true if success, false if failed
// @param[socketDat] - socket data structure to bind results to
bool socketBind(SocketDriver* const socketDat) {

	ASSERT(socketDat);											//check for valid param
	bool rv = true;												//default true, false by ERROR_IF
	int socketResult = -1; 										//default socket/stream returns invalid
	int bindResult = -1;
	ERROR_IF (false == getSocket(&socketResult));				//error if socket not acquired
	struct sockaddr* sockAddInfo = malloc(sizeof(sockaddr_in));	//allocate sockaddr info and assign data to instance
	ASSERT(NULL != sockAddInfo);								//check that buffer was successfully acquired
	(*(sockaddr_in*)sockAddInfo) = {							//assign values of struct
		g_domain,
		htons(0),
		{
			htonl(INADDR_ANY)
		},{
			0
		}
	};

	bindResult = bind(											//attempt socket binding
		socketResult,
		sockAddInfo,
		sizeof(sockAddInfo)
	);	
	ERROR_IF (bindResult < 0);									//error if socket fails binding	
	(*socketDat) = { socketResult, bindResult, sockAddInfo };	//reaching this point means success,
																//return intermediate values to input struct

	doReturn:
		
		if (false == rv) {										//if error
			if (socketResult >= 0)   { close(socketResult); }	//if socket opened but binding failed then close socket
			if (NULL != sockAddInfo) { free(sockAddInfo); }		//clean up allocated struct on error
		}
		return rv;
}

SocketDriver getSocketDriver() {
	SocketDriver rv = { -1, -1, NULL };
	socketBind(&rv);
	return rv;
}

void closeSocketDriver(SocketDriver* const s) {
	ASSERT(s);
	ASSERT(s->fd >= 0);
	close(s->fd);
}

bool clientConnect(SocketDriver* const s, int ipComponents[4]) {
	bool rv = true;													//assume success
	const int numIPComponents = 4;									//number of components in IPv4
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

void serverConnect(SocketDriver* const s) {
	
}