#include <stdio.h>
#include "SocketDriver.h"
#include "debug.h"
#include "universe.h"

int main(int argc, char**argv) {
	printf("Starting driver main().\n");
	SocketDriver driver = getSocketDriver();
	printf("SocketDriver acquired\n");
	closeSocketDriver(&driver);
	printf("Completed driver main().\n");
	return 0;
}