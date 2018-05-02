#ifndef UNIVERSE_H
	
	//compile time constants used in project
	#define UNIVERSE_H
	#define NUM_IP_COMPS		4
	#define BUF_SIZE			1500
	#define SERVER_PORT_NO		1234
	#define PATH_MAX            (1 << 10)
	#define INOT_BUF_SIZE       (sizeof(struct inotify_event) + PATH_MAX + 1)

	//types used in project
	typedef void* (*pthread_func_t)(void*);

#endif