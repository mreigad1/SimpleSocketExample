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

bool getSocket(int* const fd) {
	//check for valid params
	ASSERT(fd);

	//default true, false by ERROR_IF
	bool rv = true;

	//check that socket is acquired
	int fdesc = socket(g_domain, g_type, g_protocol);
	ERROR_IF (fdesc < 0);

	//set and return fd if socket properly acquired
	*fd = fdesc;

	doReturn:
		return rv;
}

bool socketBind(int* const fd, int* const res) {
	//check for valid params
	ASSERT(fd);
	ASSERT(res);

	//default true, false by ERROR_IF
	bool rv = true;

	//default socket/stream returns invalid
	int socketResult = -1;
	int bindResult = -1;

	//construct sockaddr info
	struct sockaddr_in sockAddInfo = { g_domain, htons(0), { htonl(INADDR_ANY) }, { 0 } };

	//error if socket not acquired
	ERROR_IF (false == getSocket(&socketResult));

	//error if socket fails binding
	bindResult = bind(socketResult, (struct sockaddr *) &sockAddInfo, sizeof(sockAddInfo));
	ERROR_IF (bindResult < 0);

	//copy return vals if valid
	*fd = socketResult;
	*res = bindResult;

	doReturn:
		//if socket opened but binding failed then close socket
		if (false == rv && socketResult >= 0) {
			close(socketResult);
		}
		return rv;
}

SocketDriver getSocketDriver() {
	SocketDriver rv = { -1, -1 };
	socketBind(&rv.fd, &rv.bindRes);
	return rv;
}

void closeSocketDriver(SocketDriver* const s) {
	ASSERT(s);
	ASSERT(s->fd >= 0);
	close(s->fd);
}