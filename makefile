all:
	gcc -std=c99 driver.c SocketDriver.c -o driver

clean:
	rm -f *.o driver