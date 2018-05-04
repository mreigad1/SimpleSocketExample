#ifndef DEBUG_H
#define DEBUG_H

	#ifndef UNIVERSE_H
		#include "universe.h"
	#endif

	#define XSTR(X) STR(X)
	#define STR(X) #X

	#define ERROR_IMPL( X , RV, HANDLER )	  \
		do {								  \
			if ( X ) {						  \
				RV ;						  \
				printf( STR(X) );			  \
				printf( " at Line = " );	  \
				printf( XSTR(__LINE__) );	  \
				printf( " in file ");		  \
				printf( XSTR(__FILE__));	  \
				printf( " with errno =  " );  \
				printf( "%d\n", errno );	  \
				HANDLER ;					  \
			}								  \
		} while(0)							  \

	#define ERROR_IF(X)	ERROR_IMPL( X,			rv = false,	goto doReturn	)
	//#define ASSERT(X)	ERROR_IMPL( false==(X),	printf("Failed Assertion.\n"),		exit(0)			)
	//#define LINE_LOG do { printf("FILE(%s):FUNC(%s):L(%d)\n", __FILE__, __func__, __LINE__); fflush(stdout); } while(0)
	#define LINE_LOG  do { ; } while(0)
	#define ASSERT(X) do { ; } while(0)

#endif
