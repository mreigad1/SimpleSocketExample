#ifndef UNIVERSE_H
#define UNIVERSE_H

	#define _BSD_SOURCE

	//compile time constants used in project
	#define NUM_IP_COMPS		4
	#define BUF_SIZE			1500
	#define SERVER_PORT_NO		1237
	#define PATH_MAX            (1 << 10)
	#define SERVER_IP			"172.20.10.2"

	#include <netinet/in.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>

	#include <errno.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <stdbool.h>

	#include <unistd.h>
	#include <string.h>
	#include <stddef.h>

	#include "debug.h"
	#include "messageTypes.h"

	//struct to contain results of
	//socket binding functions
	typedef struct {
		int fd;
		int sockoptval;
		struct sockaddr_in theirSockInfo;
		#ifdef IS_SERVER
			struct sockaddr_in mySockInfo;
		#endif
	} SocketDriver;

	//struct representing data to be passed to service routine,
	//and forwarded to method to write to all clients after population
	typedef struct {
		unsigned long bytesReceived;
		unsigned long downloadSize;
		unsigned long nextID;
		serverState_t currentState;
		CommunicationBuffer_t* data;
	} listenLoopRoutineArgs_t;

#endif
