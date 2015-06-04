#ifndef __UTILS_H__
#define __UTILS_H__

#ifndef offsetof
# define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif 

#define container_of(ptr, type, member) ({            \
 const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
 (type *)( (char *)__mptr - offsetof(type,member) );})
 
#include <stdio.h> 

#define ASSERT(condition, if_true, fmt,...) if(condition) { err_printf( #condition fmt, ##__VA_ARGS__); if_true; }

#define DO_NOTHING if(0) {}
#define DO_EXIT exit(EXIT_FAILURE)
#define DO_HUNG while(1) {};
#define DO_ABORT abort();

 
#define err_printf(fmt, ...)  fprintf(stderr, "error in %s() : %d  -- " fmt "\n\r", __func__, __LINE__ ,##__VA_ARGS__)

// #define dbg_printf(fmt, ...)  fprintf(stderr, "debug from %s() : %d  -- " fmt "\n\r", __func__, __LINE__ ,##__VA_ARGS__)
#define dbg_printf(fmt, ...)  


#define ARRSIZE(w) (sizeof(w) / sizeof(w[0]))


#define _CMP_TEMPL(a, op,b) ({ __typeof__(a) __a__ = (a); __typeof__(b) __b__ = (b); __a__ op __b__ ? __a__ : __b__; })

#define MAX(a,b) _CMP_TEMPL(a, >, b)
#define MIN(a,b) _CMP_TEMPL(a, <, b)  
#define ABS(a) ({ __typeof__(a) __a__ = (a);  __a__ > 0 ? __a__ : -((signed)__a__); })

#define lambda(rtype, body) ({ rtype __fn__ body; __fn__; })

#define likely(x)  x
#define unlikely(x)  x

//#define dbg_printf(fmt, ...)

#endif /* __UTILS_H__ */

