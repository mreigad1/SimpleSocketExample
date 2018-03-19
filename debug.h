#include <errno.h>
#include <stdio.h>

#define XSTR(X) STR(X)
#define STR(X) #X

#define ERROR_IMPL( X , RV, HANDLER )	\
	do {								\
		if ( X ) {						\
			RV ;						\
			printf( STR(X) );			\
			printf( " at Line = " );	\
			printf( XSTR(__LINE__) );	\
			printf( " with errno =  " );\
			printf( "%d\n", errno );	\
			HANDLER ;					\
		}								\
	} while(0)							\

#define ERROR_IF(X)	ERROR_IMPL( X,			rv = false,	goto doReturn	)
#define ASSERT(X)	ERROR_IMPL( false==(X),	NULL,		exit(0)			)

