#include "universe.h"
#include "SocketDriver.h"

//Only client has arguments
#ifndef IS_SERVER
	const char* serverIP = NULL;
	const char* clientFileName = NULL;
#endif

int main(int argc, char** argv) {
	#ifdef IS_SERVER
		(void) argc;
		(void) argv;
	#else
		ASSERT(3 == argc);
		serverIP = argv[1];
		clientFileName = argv[2];
	#endif

	SocketDriver mySocket = getSocketDriver();	//get initialized socket

	driverLoop(&mySocket);						//start loop to accept connections

	closeSocketDriver(&mySocket);				//close socket before exit

	return 0;
}
