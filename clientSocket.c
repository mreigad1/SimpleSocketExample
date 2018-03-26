#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/inotify.h>
#include <unistd.h>
#include "debug.h"

#include INOT_BUF_SIZE (sizeof(event) + PATH_MAX + 1)

char* outgoingFile = NULL;
char* incomingFile = NULL; 

typedef struct {
	int fd;
	int wd;
} inotifyWatcher;

typedef struct inotify_event watcherEvent;

inotifyWatcher getWatcher(char* fileName) {
	inotifyWatcher retVal;			//struct containing file and watch descriptors
	retVal.fd = inotify_init();		//file descriptor of inotify instance
	ASSERT(retVal.fd >= 0);			//assert successful inotify instantiation
	retVal.wd = inotify_add_watch(	//add watch for target file
		retVal.fd,
		fileName,
		IN_CLOSE_WRITE | IN_CREATE
	); 
	ASSERT(retVal.wd >= 0);			//assert watcher added successfully
	return retVal;
}

void handleEvent(watcherEvent* event) {
	
}

void outgoingThreadDriver() {
	inotifyWatcher watcher = getWatcher(outgoingFile);			//get watcher descriptors
	while (true) {												//loop forever
		char buffer[INOT_BUF_SIZE] = { 0 };						//stack buffer for input
		watcherEvent* event = (watcherEvent*)buffer;			//ptr for convenience
		int bytesRead = read(watch.fd, event, INOT_BUF_SIZE);	//blocking read until event 
		if (0 > bytesRead) {									//if successful read
			handleEvent(event);									//then handle event
		} else {
			printf("Error at line %d\n", __LINE__);
		}
	}
}

void incomingThreadDriver() {
	
}

int main() {

	return 0;
}