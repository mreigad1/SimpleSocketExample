#define _GNU_SOURCE

#include <pthread.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include "SocketDriver.h"
#include "debug.h"
#include "universe.h"

//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    FILE GLOBAL TYPES    **************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
#ifdef IS_SERVER
	typedef struct clientNode clientNode;		//available only for server

	struct clientNode {
		int          socketFD;					//file descriptor of this socket
		pthread_t    servicingThread;			//thread servicing this client
		clientNode*  next;						//next client in list
	};

	typedef struct {
		void* forwardedArgs;
		pthread_func_t serviceRoutine;
	} launcherArgs;
#endif

//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    FUNCTION PROTOTYPES    ************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
bool getSocket(int* const fd, int* const sockoptval);
bool socketBind(SocketDriver* const s);

#ifdef IS_SERVER
	void serverServiceAccept(const int fd, pthread_func_t serviceRoutine);
	void* nonblockingWrite(void* argData);
	void* routineLauncher(void* args);
#endif

//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    FILE GLOBAL VARIABLES    **********************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
static const int g_domain = AF_INET;
static const int g_type = SOCK_STREAM | SOCK_NONBLOCK;
static const int g_protocol = 0;
static const int g_bufferSize = 4096;

#ifdef IS_SERVER
	size_t      clientCount = 0;
	clientNode* clientList = NULL;		//available only for server
	pthread_mutex_t clientListLock = PTHREAD_MUTEX_INITIALIZER;
	clientNode* freeClient = NULL;
#endif

//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    GENERAL DRIVER CODE    ************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
SocketDriver getSocketDriver(void) {
	SocketDriver rv = { 0 };			//if field later unspecified, default all zeroes
	rv.fd = -1;							//specify each field with
	rv.sockoptval = -1;
	rv.socketData = NULL;
	socketBind(&rv);
	return rv;
}

void closeSocketDriver(SocketDriver* const s) {
	ASSERT(s);
	ASSERT(s->fd >= 0);
	close(s->fd);
	if (s->socketData) {
		LINE_LOG;
		free(s->socketData);
		s->socketData = NULL;
	}
}

// @Procedure - opens socket and provides file descriptor
// @return - true if success, false if failed
// @param[fd] - pointer to int where file descriptor of socket
//              should be written, does not write on failure
// @param[sockoptval] - pointer to sockOptVal buffer when
//                      invoking upon server
bool getSocket(int* const fd, int* const sockoptval) {
	ASSERT(fd);							//check for valid params
	ASSERT(sockoptval);
	bool rv = true;						//default true, false by ERROR_IF
	int fdesc = socket(					//check that socket is acquired
		g_domain,
		g_type,
		g_protocol
	);
	ERROR_IF (fdesc < 0);				//error when opening socket fails
	#ifdef IS_SERVER					//following section only when server
		*sockoptval = 1;				//parameter for setsockopt
		int serverOptRes = setsockopt(
			fdesc,
			SOL_SOCKET,
			SO_REUSEADDR,
			sockoptval,
			sizeof(*sockoptval)
		);
		ERROR_IF (serverOptRes != 0);	//error is server opt
	#endif
	*fd = fdesc;						//set and return fd if socket properly acquired
	doReturn:
		if (false == rv) {				//on error flag sockoptval negative
			*sockoptval = -1;
		}
		return rv;
}

// @Procedure - opens and binds socket, provides file
//              descriptor and bind result
// @return - true if success, false if failed
// @param[s] - socket data structure to bind results to
bool socketBind(SocketDriver* const s) {
	ASSERT(s);													//check for valid param
	bool rv = true;												//default true, false by ERROR_IF
	int socketResult = -1; 										//default socket/stream returns invalid
	int bindResult = -1;
	bool getSocketSuccess = getSocket(							//get socket file descriptor and set opt val
		&socketResult,
		&s->sockoptval
	);
	ERROR_IF (false == getSocketSuccess);						//error if socket not acquired
	struct sockaddr_in* sockInfo = malloc(						//allocate sockaddr info and assign data to instance
		sizeof(struct sockaddr_in)
	);
	ASSERT(NULL != sockInfo);									//check that buffer was successfully acquired
	sockInfo->sin_family = g_domain;							//assign values
	sockInfo->sin_addr.s_addr = htonl(INADDR_ANY);
	#ifdef IS_SERVER
		int portToUse = SERVER_PORT_NO;							//if server then use server port num
	#else
		int portToUse = 0;										//else use OS assigned port
	#endif
	sockInfo->sin_port = htons(portToUse);

	bindResult = bind(											//attempt socket binding
		socketResult,
		(struct sockaddr*)sockInfo,
		sizeof(*sockInfo)
	);
	ASSERT(0 == bindResult);									//error if socket fails binding	
	s->fd = socketResult;										//reaching this point means success,
	s->socketData = (struct sockaddr*)sockInfo;

	doReturn:
		
		if (false == rv) {										//if error
			if (socketResult >= 0) { close(socketResult); }		//if socket opened but binding failed then close socket
			if (NULL != sockInfo)  { 
				LINE_LOG;
				free(sockInfo);
			}													//clean up allocated struct on error
		}
		return rv;
}

//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    CLIENT DRIVER CODE    *************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
#ifndef IS_SERVER
	bool clientConnect(SocketDriver* const s, int ipComponents[NUM_IP_COMPS]) {
		ASSERT(s);														//verify inputs valid
		ASSERT(NULL == s->serverData);
		ASSERT(ipComponents);
		bool rv = true;													//assume success
		struct sockaddr_in* tmp_sock_in = malloc(						//allocate server socket addr
			sizeof(struct sockaddr_in)
		);
		memset(tmp_sock_in, 0, sizeof(struct sockaddr_in));				//wipe server socket addr
		tmp_sock_in->sin_family = g_domain;
		tmp_sock_in->sin_port = htons(SERVER_PORT_NO);
		unsigned char ipCastdown[NUM_IP_COMPS] = {	 					//convenient casting array for copying ip address
			ipComponents[0],
			ipComponents[1],
			ipComponents[2],
			ipComponents[3]
		};
		void* sin_addr = &(tmp_sock_in->sin_addr);						//take address of buffer for memcpy
		memcpy(sin_addr, ipCastdown, NUM_IP_COMPS);						//copy ip address into struct
		int connectRet = connect(
			s->fd,
			(struct sockaddr *)tmp_sock_in,
			sizeof(struct sockaddr_in)
		);

		ASSERT(0 == connectRet || EINPROGRESS == errno);

		struct pollfd myfd = {
			s->fd,
			POLLIN | POLLOUT | POLLPRI,
			0
		};

		connectRet = poll(
			&myfd,														//fd struct to poll
			1,															//send one fd
			-1															//poll forever if necessary
		);

		ASSERT(1 == connectRet);
		if (1 != connectRet) {	
			memset(sin_addr, 0, NUM_IP_COMPS);							//on failure to connect, wipe buffer
			rv = false;													//flag ret val to failure
			LINE_LOG;
			free(tmp_sock_in);											//release allocated block
		} else {
			s->serverData = (struct sockaddr *)tmp_sock_in;
		}
		return rv;
	}
#endif

//*****************************************************************************************************************
//*****************************************************************************************************************
//**************************************    SERVER DRIVER CODE    *************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
#ifdef IS_SERVER
	bool serverSetListen(SocketDriver* const s) {
		ASSERT(s);														//verify input valid
		const int backlog = 5;											//number of backlogged connections permitted on socket
		bool rv = true;													//default success
		ERROR_IF(listen(s->fd, backlog) != 0);							//error on failed listen
		doReturn:
			return rv;
	}

	void* nonblockingWrite(void* argData) {
		listenLoopRoutineArgs_t* const arg = argData;					//cast to struct type
		write(arg->fd, arg->outData.data, arg->outData.dataLength);		//write data to socket
		LINE_LOG;
		free(arg);														//allocated in writeToAllClients(...)
		return NULL;
	}

	void writeToAllClients(listenLoopRoutineArgs_t* const outgoing) {
		ASSERT(outgoing);												//check input valid
		size_t indexCount = 0;											//counter to index threads
		pthread_mutex_lock(&clientListLock);
		LINE_LOG;
		const int myClientCount = clientCount;
		pthread_t servicingThread[clientCount];							//threads to serve data to clients
		pthread_func_t writeRoutine = nonblockingWrite;					//routine to fork threads into
		clientNode* outClient = clientList;
		while (NULL != outClient) {										//for each client subscribed
			LINE_LOG;
			listenLoopRoutineArgs_t* const clientArgData = malloc(		//to be cleaned up in nonblockingWrite(...)
				sizeof(listenLoopRoutineArgs_t)
			);
			clientArgData->fd = outClient->socketFD;					//take FD of client node
			clientArgData->outData = outgoing->outData;					//take outgoing data
			LINE_LOG;
			pthread_create(												//fork new thread to serve client new data
				&servicingThread[indexCount++],
				NULL,
				writeRoutine,
				(void*)clientArgData
			);
			LINE_LOG;
			outClient = outClient->next;								//iterate to next client
		}
		pthread_mutex_unlock(&clientListLock);
		LINE_LOG;
		for (indexCount = 0; indexCount < myClientCount; indexCount++) {//join up thread for each client
			pthread_join(servicingThread[indexCount], NULL);
		}
		LINE_LOG;
	}

	void serverListenLoop(SocketDriver* const s, pthread_func_t serviceRoutine) {
		ASSERT(s);														//check valid inputs
		ASSERT(serviceRoutine);
		LINE_LOG;
		const int pumpTheBreaks_ms = 40;
		while (true) {													//loop endlessly
			int requestDescriptor = -1;									//file descriptor of next issued accept
			socklen_t addrLen = sizeof(struct sockaddr_in);				//take length of addr struct
			do {														//attempt accept until new connection
				LINE_LOG;
				requestDescriptor = accept4(							//attempt to accept next
					s->fd,
					s->socketData,
					&addrLen,
					SOCK_NONBLOCK
				);
				if (requestDescriptor < 0) {							//broke accept for error 
					const int error = errno;							//capture error number
					static const int acceptableErrors[] = {
						ECHILD,
						ERESTART,
						EINTR,
						EAGAIN
					};													//list of errors tolerated
					LINE_LOG;
					const int arrSize =	
						sizeof(acceptableErrors) / sizeof(int);			//take number of tolerated errors
					int counter = 0;
					bool isAcceptableError = false;
					for (; counter < arrSize; counter++) {				//if tolerated error then flag appropriately
						if (acceptableErrors[counter] == error) {
							isAcceptableError = true;
						}
					}
					LINE_LOG;
					if (false == isAcceptableError) {					//if not tolerated error then display error and die
						fprintf(
							stderr,
							"accept failed for error number: %d\n",
							error
						);
						exit(1);
					}
					LINE_LOG;
				}
				usleep(pumpTheBreaks_ms * 1000);
			} while (requestDescriptor < 0);
			LINE_LOG;
			serverServiceAccept(requestDescriptor, serviceRoutine);		//new valid accept received, service it
			usleep(pumpTheBreaks_ms * 1000);
		}
	}

	void* routineLauncher(void* args) {
		ASSERT(args);
		launcherArgs* largs = args;
		pthread_func_t serviceRoutine = largs->serviceRoutine;
		listenLoopRoutineArgs_t* fargs = largs->forwardedArgs;
		free(args);														//allocated in serviceAcceptRoutine

		serviceRoutine(fargs);											//forward arguments to user routine

		pthread_mutex_lock(&clientListLock);							//on return from user routine
		if (freeClient) {												//cleanup old client no longer running
			pthread_join(freeClient->servicingThread, NULL);			//must have added itself, should be ready to join
			free(freeClient);											//free its memory block
			freeClient = NULL;
		}

		if (NULL != clientList) {
			if (clientList->socketFD == fargs->fd) {					//if fd to remove is at head of list
				LINE_LOG;
				freeClient = clientList;								//move bad node to freeClient
				clientList = clientList->next;							//advance head node of clientList
				LINE_LOG;
			} else {
				LINE_LOG;
				clientNode* tmp = clientList;							//otherwise iterate clientList for file descriptor
				while (NULL != tmp->next &&
					fargs->fd != tmp->next->socketFD) {					//or until end of list
						LINE_LOG;
						tmp = tmp->next;
				}
				LINE_LOG;
				if (NULL != tmp->next &&
					fargs->fd == tmp->next->socketFD) {					//if file descriptor found, then
						LINE_LOG;
						close(fargs->fd);								//close descriptor
						freeClient = tmp->next;							//move node to freeClient
						tmp->next = freeClient->next;					//move parent to point at granchild node
						freeClient->next = NULL;						//wipe list remainder from removed node
						clientCount--;									//decrement client count
						LINE_LOG;
				}
			}
		}
		pthread_mutex_unlock(&clientListLock);
		free(fargs);													//buffer handed to thread must be cleared
		LINE_LOG;
		return NULL;
	}

	// @Procedure - receives file descriptor returned from accept()
	//              services and manages newly accepted socket
	// @return - None
	// @param[fd] - file descriptor of new request
	// @param[serviceRoutine] - routine to fork pthread accept service threads into
	void serverServiceAccept(const int fd, pthread_func_t serviceRoutine) {
		ASSERT(fd >= 0);									//check valid inputs
		ASSERT(serviceRoutine);
		clientNode* newNode = malloc(sizeof(clientNode));	//allocate new clientNode
		newNode->socketFD = fd;								//set file descriptor
		LINE_LOG;
		pthread_mutex_lock(&clientListLock);
		newNode->next = clientList;
		clientList = newNode;								//move new clientNode to head of clientList
		clientCount++;
		pthread_mutex_unlock(&clientListLock);
		LINE_LOG;
		listenLoopRoutineArgs_t* argData = malloc(			//to be deallocated int routineLauncher(...)
			sizeof(listenLoopRoutineArgs_t)
		);													//allocate struct for argData, cleaned up in consumer thread
		argData->fd = fd;
		LINE_LOG;
		launcherArgs* launcherContainer = malloc(			//to be deallocated in routineLauncher(...)
			sizeof(launcherArgs)
		);
		launcherContainer->serviceRoutine = serviceRoutine;
		launcherContainer->forwardedArgs = argData;
		LINE_LOG;
		pthread_create(										//fork new thread to serve client
			&newNode->servicingThread,
			NULL,
			routineLauncher,								//launches callback and manages cleanup from client pool
			launcherContainer								//container storing callback and arguments
		);
		LINE_LOG;
	}
#endif