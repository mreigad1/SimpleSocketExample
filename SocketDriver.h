#include <stdbool.h>
#include "universe.h"

//declare opaque struct type for sock data ptr
struct sockaddr;

//struct to contain results of
//socket binding functions
typedef struct {
	int fd;
	int bindRes;
	int sockoptval;
	struct sockaddr* socketData;
} SocketDriver;

// @Procedure - opens socket and returns socket data
// @return - struct containing socket information
//           on failure, all struct fields <= 0
SocketDriver getSocketDriver();

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
	// @Procedure - sets socket to listen for
	//              incoming connections
	// @return - true on success, false on failure
	bool serverSetListen(SocketDriver* const s);

	// @Procedure - enters server into listening state
	//              so that server may begin accepting
	//              and servicing connections
	// @return - TBD
	void serverListenLoop(SocketDriver* const s);
#endif