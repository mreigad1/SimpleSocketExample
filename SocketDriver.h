#include "universe.h"

// @Procedure - opens socket and returns socket data
// @return - struct containing socket information
//           on failure, all struct fields <= 0
SocketDriver getSocketDriver(void);

// @Procedure - closes socket
// @param[s] - socket driver struct containing
//             data for socket to be closed
void closeSocketDriver(SocketDriver* const s);

// @Procedure - enters server into listening state
//              so that server may begin accepting
//              and servicing connections
// @param[s] - socket driver struct to drive the listen loop
// @return - TBD
void driverLoop(SocketDriver* const s);

