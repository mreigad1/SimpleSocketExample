all:
	gcc -std=c99 main.c SocketDriver.c server.c client.c -o driver

clean:
	rm -f *.o driver