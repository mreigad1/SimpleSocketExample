
SOCKET_TEST=test_driver
READER_APP=reader
PROGRAM_LIST=${SOCKET_TEST} ${READER_APP}

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
	USER_NAME=$(shell bash -c 'cat ${USER_FILE}')

${SOCKET_TEST}:
	gcc -std=c99 main.c SocketDriver.c server.c client.c -o ${SOCKET_TEST}

${READER_APP}:
	gcc -std=c99 reader.c -o ${READER_APP}

${CLIENT_APP}: default_user ${READER_APP}
	USER_NAME=$(shell bash -c 'cat ${USER_FILE}')
	${NEW_TERMINAL} ${NEW_TERM_CMD}"./${READER_APP} ${OUTGOING_FILE} matt"


clean:
	rm -f *.o ${CLEANUP_LIST}