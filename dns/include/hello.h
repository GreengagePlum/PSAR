#ifndef HELLO_H
#define HELLO_H

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC const char *hello(void);

#undef EXTERNC

#endif /* HELLO_H */
