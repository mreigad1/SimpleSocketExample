
#include <stdbool.h>

typedef struct {
	int fd;
	int bindRes;
} SocketDriver;

bool getSocket(int* const fd);

bool socketBind(int* const fd, int* const res);

SocketDriver getSocketDriver();
