#include <stdbool.h>

//struct to contain results of
//socket binding functions
typedef struct {
	int fd;
	int bindRes;
} SocketDriver;

// @Procedure - opens socket and provides file descriptor
// @return - true if success, false if failed
// @param[fd] - pointer to int where file descriptor of socket
//              should be written, does not write on failure
bool getSocket(int* const fd);

// @Procedure - opens and binds socket, provides file
//              descriptor and bind result
// @return - true if success, false if failed
// @param[fd] - pointer to int where file descriptor of socket
//              should be written, does not write on failure
// @param[res] - pointer to int where result of socket binding
//               should be written, does not write on failure
bool socketBind(int* const fd, int* const res);

// @Procedure - opens socket and returns socket data
// @return - struct containing socket information
SocketDriver getSocketDriver();

// @Procedure - closes socket
// @param[s] - socket driver struct containing
//             data for socket to be closed
void closeSocketDriver(SocketDriver* const s);