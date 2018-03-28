
SOCKET_TEST=test_driver
CLIENT_INPUT_PROG=clientInput
CLIENT_SOCKET_PROG=clientSocket
PROGRAM_LIST=${SOCKET_TEST} ${CLIENT_INPUT_PROG} ${CLIENT_SOCKET_PROG} ${SERVER_APP}

COMPILATION_FLAGS=-Wall -Wextra -std=c99 -pedantic -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition

CLIENT_APP=client
SERVER_APP=server
INCOMING_FILE=incF.txt
OUTGOING_FILE=outF.txt

USER_FILE=userName.conf
USER_NAME=""
CLEANUP_LIST=${PROGRAM_LIST} ${USER_FILE}

NEW_TERMINAL=gnome-terminal
NEW_TERM_CMD=--command=

${SOCKET_TEST}:
	gcc ${COMPILATION_FLAGS} main.c SocketDriver.c server.c client.c -o ${SOCKET_TEST}

${CLIENT_INPUT_PROG}:
	gcc ${COMPILATION_FLAGS} ${CLIENT_INPUT_PROG}.c -o ${CLIENT_INPUT_PROG}

${CLIENT_SOCKET_PROG}:
	gcc ${COMPILATION_FLAGS} -pthread ${CLIENT_SOCKET_PROG}.c SocketDriver.c -lpthread -o ${CLIENT_SOCKET_PROG}

default_user:
	./setUser.sh User_$(shell bash -c 'echo $$RANDOM') ${USER_FILE}

${CLIENT_APP}: default_user ${CLIENT_INPUT_PROG} ${CLIENT_INPUT_PROG}
	${NEW_TERMINAL} ${NEW_TERM_CMD}"./${CLIENT_INPUT_PROG} ${OUTGOING_FILE} $(shell bash -c 'cat ${USER_FILE}')"

${SERVER_APP}:
	gcc ${COMPILATION_FLAGS} -DIS_SERVER -pthread ${SERVER_APP}.c SocketDriver.c -lpthread -o ${SERVER_APP}
	./server

clean:
	rm -f *.o ${CLEANUP_LIST}