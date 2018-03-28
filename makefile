
SOCKET_TEST=test_driver
READER_PROG=reader
WRITER_PROG=writer
CLIENT_SOCKET_PROG=clientSocket
PROGRAM_LIST=${SOCKET_TEST} ${READER_PROG} ${WRITER_PROG} ${CLIENT_SOCKET_PROG}

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

default_user:
	./setUser.sh User_$(shell bash -c 'echo $$RANDOM') ${USER_FILE}

${SOCKET_TEST}:
	gcc ${COMPILATION_FLAGS} main.c SocketDriver.c server.c client.c -o ${SOCKET_TEST}

${READER_PROG}:
	gcc ${COMPILATION_FLAGS} ${READER_PROG}.c -o ${READER_PROG}

${CLIENT_SOCKET_PROG}:
	gcc ${COMPILATION_FLAGS} -pthread ${CLIENT_SOCKET_PROG}.c -lpthread -o ${CLIENT_SOCKET_PROG}

${WRITER_PROG}:
	gcc ${COMPILATION_FLAGS} ${WRITER_PROG}.c -o ${WRITER_PROG}

${CLIENT_APP}: default_user ${READER_PROG}
	${NEW_TERMINAL} ${NEW_TERM_CMD}"./${READER_PROG} ${OUTGOING_FILE} $(shell bash -c 'cat ${USER_FILE}')"


clean:
	rm -f *.o ${CLEANUP_LIST}