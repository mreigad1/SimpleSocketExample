#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <pthread.h>
#include "debug.h"

#define PATH_MAX (1 << 10)
#define INOT_BUF_SIZE (sizeof(struct inotify_event) + PATH_MAX + 1)

char* outgoingFile = NULL;
char* incomingFile = NULL; 

typedef struct {
	int fd;
	int wd;
} inotifyWatcher;

typedef struct inotify_event watcherEvent;

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

void handleEvent(watcherEvent* event) {
	printf("Event received.\n");
	fflush(stdout);
}

void outgoingThreadDriver() {
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

void incomingThreadDriver() {
	
}

//argv[0] prog name
//argv[1] outgoingFile name
//argv[2] incomingFile name
int main(int argc, char** argv) {
	outgoingFile = argv[1];
	incomingFile = argv[2];

	ASSERT(3 == argc);
	ASSERT(NULL != argv[1]);
	ASSERT(NULL != argv[2]);

	outgoingThreadDriver();

	return 0;
}