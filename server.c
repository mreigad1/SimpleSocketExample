#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "SocketDriver.h"
#include "debug.h"
#include "universe.h"

void* onNewConnection(void* fd);

void* onNewConnection(void* fd) {
	ASSERT(fd);

	while (true) {
		printf("received new connection with file descriptor: %d\n", *((int*)fd));
		sleep(20);
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