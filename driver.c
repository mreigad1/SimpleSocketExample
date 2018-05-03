#include "universe.h"
#include "SocketDriver.h"

int main(void) {

	SocketDriver mySocket = getSocketDriver();	//get initialized socket

	driverLoop(&mySocket);						//start loop to accept connections

	closeSocketDriver(&mySocket);				//close socket before exit

	return 0;
}