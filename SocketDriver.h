#include <stdbool.h>
#include "universe.h"

//declare opaque struct type for sock data ptr
struct sockaddr;

//struct to contain results of
//socket binding functions
typedef struct {
	int fd;
	int sockoptval;
	struct sockaddr* socketData;
	#ifndef IS_SERVER
		struct sockaddr* serverData;
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

#ifndef IS_SERVER
	// @Procedure - issues connection from client to server
	// @return - true on success, false on failure
	bool clientConnect(SocketDriver* const s, int ipComponents[NUM_IP_COMPS]);
#endif

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
	} listenLoopRoutineArgs_t;

	// @Procedure - sets socket to listen for
	//              incoming connections
	// @return - true on success, false on failure
	void writeToAllClients(listenLoopRoutineArgs_t* const outgoing);

	// @Procedure - sets socket to listen for
	//              incoming connections
	// @return - true on success, false on failure
	bool serverSetListen(SocketDriver* const s);

	// @Procedure - enters server into listening state
	//              so that server may begin accepting
	//              and servicing connections
	// @param[s] - socket driver struct to drive the listen loop
	// @param[serviceRoutine] - pthread_func_t type procedure to forward new accepted connections to.
	//                          listenLoopRoutineArgs_t to be passed to serviceRoutine containing fd
	//                          of the client writing to server, rest of struct to be populated by
	//                          serviceRoutine before forwarding to writeToAllClients
	// @return - TBD
	void serverListenLoop(SocketDriver* const s, pthread_func_t serviceRoutine);
#endif