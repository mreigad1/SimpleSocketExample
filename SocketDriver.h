#include <stdbool.h>
#include "universe.h"
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

// @Procedure - opens socket and returns socket data
// @return - struct containing socket information
//           on failure, all struct fields <= 0
SocketDriver getSocketDriver(void);

// @Procedure - closes socket
// @param[s] - socket driver struct containing
//             data for socket to be closed
void closeSocketDriver(SocketDriver* const s);

#ifdef IS_SERVER
	//struct representing data to be written to all client sockets
	typedef struct {
		void* data;
		size_t dataLength;
	} writeData_t;

	//struct representing data to be passed to service routine,
	//and forwarded to method to write to all clients after population
	typedef struct {
		int fd;
		writeData_t outData;
		serverState_t currentState;
		unsigned long bytesReceived;
		unsigned long downloadSize;
		unsigned long nextID;
	} listenLoopRoutineArgs_t;

	// @Procedure - enters server into listening state
	//              so that server may begin accepting
	//              and servicing connections
	// @param[s] - socket driver struct to drive the listen loop
	// @return - TBD
	void serverListenLoop(SocketDriver* const s);
#else

	// @Procedure - enters server into listening state
	//              so that server may begin accepting
	//              and servicing connections
	// @param[s] - socket driver struct to drive the listen loop
	// @return - TBD
	void clientSendLoop(SocketDriver* const s);

#endif