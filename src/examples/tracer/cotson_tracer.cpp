// This is a simple example of an application (to run in the guest)
// that invokes simulation hooks (handler defined in tracer_example.cpp).
//
// The call to COTSON_INTERNAL() [defined in cotson_tracer.h] uses
// a backdoor to invoke the simulator with some simulated-world parameters.
// The simulator can then use these parameters to triggers different 
// behavior (for example sampling), examine simulated memory or data
// structures, and so on.
//

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "cotson_tracer.h"

int main(int argc, char *argv[])
{
    char* sbuf = (char *)malloc(20);
	uint64_t a=0;
	uint64_t b=0;
	uint64_t cp=0;

	a=atoll(argv[1]);
	b=atoll(argv[2]);
    sprintf(sbuf,"hello world %s",argv[3]); cp=(uint64_t)sbuf;

// Pass two argc parameters by value and the third as a pointer
// to some simulated memory (a string buf in this example)
    printf("%s called with %ld %ld %ld\n", argv[0],a,b,cp);
	COTSON_INTERNAL(a,b,cp);

	free(sbuf);
	return 0;
}
