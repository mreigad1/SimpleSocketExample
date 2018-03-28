#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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
#define ASSERT(X)	ERROR_IMPL( false==(X),	NULL,		exit(0)			)

#define LINE_LOG do { printf("Line at %d\n", __LINE__); fflush(stdout); } while(0)

