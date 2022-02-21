#ifndef _WDEBUG_H_
#define _WDEBUG_H_

#ifdef _WINDOWS
#ifdef __cplusplus
extern "C"
#endif
void WINDEBUG(char *c, int t, int w, int h, int s, unsigned char *b);
#else
#define	WINDEBUG(c, t, w, h, s, b)
#endif

#endif // _WDEBUG_H_
