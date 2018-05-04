#include "universe.h"
#include "SocketDriver.h"

const char* serverIP = NULL;

#ifndef IS_SERVER
	const char* clientFileName = NULL;
#endif

int main(int argc, char** argv) {
	#ifdef IS_SERVER
		ASSERT(2 == argc);
	#else
		ASSERT(3 == argc);
		clientFileName = argv[2];
	#endif

	serverIP = argv[1];

	SocketDriver mySocket = getSocketDriver();	//get initialized socket

	driverLoop(&mySocket);						//start loop to accept connections

	closeSocketDriver(&mySocket);				//close socket before exit

	return 0;
}
