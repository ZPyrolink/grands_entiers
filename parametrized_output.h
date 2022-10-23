#ifndef PARAMETRIZED_OUTPUT_H
#define PARAMETRIZED_OUTPUT_H

#ifdef VERBOSE
#define alert(f,...) fprintf(stderr, "*** WARNING *** " f, __VA_ARGS__)
#else
#define alert(f,...)
#endif

#ifdef DEBUG
#define debug(f,...) fprintf(stderr, "*** DEBUG *** " f, __VA_ARGS__)
#else
#define debug(f,...)
#endif

#endif
