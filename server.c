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

void* onNewConnection(void* args);

//routine specifies what should be done when new connection is received
//will be executed on separate asynchronous client servicing thread
void* onNewConnection(void* args) {
	ASSERT(args);

	const int clientPollingRate_ms = 250;
	char buffer[BUF_SIZE];															//stack buffer for reading client data
	listenLoopRoutineArgs_t* dat = args;											//take args as struct
	dat->outData.data = buffer;														//note buffer location
	dat->outData.dataLength = BUF_SIZE;												//note buffer size
	printf("received new connection with file descriptor: %d\n", dat->fd);			//notify of new connection
	while (true) {																	//loop endlessly
		memset(buffer, 0, sizeof(buffer));											//wipe buffer
		while (read(dat->fd, dat->outData.data, sizeof(buffer) - 1) < 0) {			//read into buffer
			LINE_LOG;
			ASSERT(EAGAIN == errno);												//assert error was EAGAIN
			LINE_LOG;
			usleep(clientPollingRate_ms * 1000);									//sleep until next read attempt
			LINE_LOG;
		}
		LINE_LOG;
		if (EPIPE != errno) {
			LINE_LOG;
			writeToAllClients(dat);													//read successful, write data
			LINE_LOG;
		} else {
			LINE_LOG;
			break;																	//break when connection closed
		}
	}

	return NULL;
}

int main(int argc, char** argv) {
	ASSERT(argv);									//check inputs
	ASSERT(argc > 0);

	SocketDriver mySocket = getSocketDriver();		//get initialized socket
	serverSetListen(&mySocket);						//begin listening
	serverListenLoop(&mySocket, onNewConnection);	//start loop to accept connections
	closeSocketDriver(&mySocket);					//close socket before exit

	return 0;
}