#define _BSD_SOURCE

#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "SocketDriver.h"
#include "debug.h"
#include "universe.h"
#include "messageTypes.h"

int main(int argc, char** argv) {
	ASSERT(argv);									//check inputs
	ASSERT(argc > 0);

	SocketDriver mySocket = getSocketDriver();		//get initialized socket
	serverSetListen(&mySocket);						//begin listening
	serverListenLoop(&mySocket, onNewConnection);	//start loop to accept connections
	closeSocketDriver(&mySocket);					//close socket before exit

	return 0;
}