#include <stdio.h>
#include "SocketDriver.h"
#include "debug.h"

int main() {
	printf("Starting driver main().\n");
	SocketDriver driver = getSocketDriver();

	printf("\nCompleted driver main().\n");
	return 0;
}