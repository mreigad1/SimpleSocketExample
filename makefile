
SOCKET_TEST=test_driver
READER_PROG=reader
WRITER_PROG=reader
PROGRAM_LIST=${SOCKET_TEST} ${READER_PROG}

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
	gcc -std=c99 main.c SocketDriver.c server.c client.c -o ${SOCKET_TEST}

${READER_PROG}:
	gcc -std=c99 ${READER_PROG}.c -o ${READER_PROG}

${WRITER_PROG}:
	gcc -std=c99 ${WRITER_PROG}.c -o ${WRITER_PROG}

${CLIENT_APP}: default_user ${READER_PROG}
	${NEW_TERMINAL} ${NEW_TERM_CMD}"./${READER_PROG} ${OUTGOING_FILE} $(shell bash -c 'cat ${USER_FILE}')"


clean:
	rm -f *.o ${CLEANUP_LIST}