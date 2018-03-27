#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <pthread.h>
#include "debug.h"

//maximum expected path length and max size of inotify buffer to read
#define PATH_MAX (1 << 10)
#define INOT_BUF_SIZE (sizeof(struct inotify_event) + PATH_MAX + 1)

char* outgoingFile = NULL;
char* incomingFile = NULL; 
char* serverIPAddress = NULL;
const int numIPComps = 4;
unsigned int ipComponents[numIPComps] = { -1, -1, -1, -1 };

typedef struct {
	int fd;
	int wd;
} inotifyWatcher;

typedef struct inotify_event watcherEvent;

//function gets struct instance of user input watcher
inotifyWatcher getWatcher(char* fileName) {
	inotifyWatcher retVal;				//struct containing file and watch descriptors
	retVal.fd = inotify_init();			//file descriptor of inotify instance
	ASSERT(retVal.fd >= 0);				//assert successful inotify instantiation
	ASSERT(NULL != outgoingFile);		//assert outgoingFile has been specified
	fclose(fopen(outgoingFile, "w"));	//touch & clear outgoingFile
	retVal.wd = inotify_add_watch(		//add watch for target file
		retVal.fd,
		fileName,
		IN_CLOSE_WRITE | IN_CREATE
	);
	ASSERT(retVal.wd >= 0);				//assert watcher added successfully
	return retVal;
}

//function handles notification when user input file has been updated
void handleEvent(watcherEvent* event) {
	
}

//driver function for thread
//managing outgoing socket comms
void outgoingThreadDriver(void* unused) {
	inotifyWatcher watcher = getWatcher(outgoingFile);			//get watcher descriptors
	while (true) {												//loop forever
		char buffer[INOT_BUF_SIZE] = { 0 };						//stack buffer for input
		watcherEvent* event = (watcherEvent*)buffer;			//ptr for convenience
		int bytesRead = read(watcher.fd, event, INOT_BUF_SIZE);	//blocking read until event 
		if (0 <= bytesRead) {									//if successful read
			handleEvent(event);									//then handle event
		} else {
			printf("Error at line %d, bytesRead = %d, sizeof(watcherEvent) = %d\n", __LINE__, bytesRead, (int)sizeof(watcherEvent));
		}
	}
}

//driver function for thread
//managing incoming socket comms
void incomingThreadDriver(void* unused) {

}

//driven by main to parse input args
//argv[0] prog name
//argv[1] outgoingFile name
//argv[2] incomingFile name
//argv[3] server IP address
void parseCmdArgs(int argc, char** argv) {
	outgoingFile = argv[1];
	incomingFile = argv[2];
	serverIPAddress = arg[3];

	ASSERT(3 == argc);
	ASSERT(NULL != argv[1]);
	ASSERT(NULL != argv[2]);
	ASSERT(NULL != argv[3]);

	int ipCompCounter = 0;
	char* subComp = strtok(serverIPAddress, ".");
	while (subComp != NULL) {
		ASSERT(numIPComps > ipCompCounter);
		ipComponents[ipCompCounter++] = atoi(subComp);
		subComp = strtok(NULL, ".");
	}

	for (ipCompCounter = 0; ipCompCounter < numIPComps; ipCompCounter++) {
		ASSERT(0 <= ipComponents[ipCompCounter]);
	}
}

void deployThreads() {
	pthread_t outgoingThread;
	//pthread_t incomingThread;

 	/* create threads 1 and 2 */    
    pthread_create (&outgoingThread, NULL, (void *) &outgoingThreadDriver, NULL);
    //pthread_create (&incomingThread, NULL, (void *) &incomingThreadDriver, NULL);

    //put main thread to sleep while children threads handle execution
    //while(1) { sleep(10); }

    incomingThreadDriver(NULL);
}

int main(int argc, char** argv) {
	parseCmdArgs(argc, argv);
	deployThreads();
	return 0;
}