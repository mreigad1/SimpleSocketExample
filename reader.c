#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "universe.h"

char globalBuf[BUF_SIZE + 1];

void getMessage() {
	//wipe buffer
	memset(globalBuf, 0, BUF_SIZE + 1);

	//read input from stdin
	fgets(globalBuf, BUF_SIZE, stdin);
}

//argv[0] prog name
//argv[1] outfile name
//argv[2] user name
int main(int argc, char** argv) {

	//handle for buffer to be allocated in function call
	char* outFileName = argv[1];
	char* userName = argv[2];

	ASSERT(3 == argc);
	ASSERT(NULL != argv[1]);
	ASSERT(NULL != argv[2]);

	//read input indefinitely
	while (true) {

		//show user their prompt
		printf("%s> ", userName);

		//read buffer from input
		getMessage();

		//open and stomp on outgoing message file
		FILE* outFile = fopen(outFileName, "w");

		//display input
		fprintf(outFile, "%s> %s", userName, globalBuf);

		//close file
		fclose(outFile);
	}

	return 0;
}