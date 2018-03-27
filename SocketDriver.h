#include <stdbool.h>

//declare opaque struct type for sock data ptr
struct sockaddr;

//struct to contain results of
//socket binding functions
typedef struct {
	int fd;
	int bindRes;
	struct sockaddr* socketData;
} SocketDriver;

// @Procedure - opens socket and returns socket data
// @return - struct containing socket information
SocketDriver getSocketDriver();

// @Procedure - closes socket
// @param[s] - socket driver struct containing
//             data for socket to be closed
void closeSocketDriver(SocketDriver* const s);

// @Procedure - issues connection from client to server
// @return - true on success, false on failure
bool clientConnect(SocketDriver* const s, int ipComponents[4]);

// @Procedure - listens for incoming connections
// @return - TBD
void serverConnect(SocketDriver* const s);