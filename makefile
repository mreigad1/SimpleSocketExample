CLIENT_APP=client
SERVER_APP=server

COMPILATION_FLAGS=-Werror -Wall -Wextra -std=c99 -pedantic -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition -Wincompatible-pointer-types

CLEANUP_LIST==${CLIENT_APP} ${SERVER_APP}

${CLIENT_APP}:
	gcc ${COMPILATION_FLAGS} driver.c SocketDriver.c clientHandler.c -o ${CLIENT_APP}

${SERVER_APP}:
	gcc ${COMPILATION_FLAGS} -DIS_SERVER driver.c SocketDriver.c messageHandler.c -o ${SERVER_APP}

all:

clean:
	rm -f *.o client server