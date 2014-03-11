/* Copyright (c) 1988,1992 NeXT, Inc. - 9/8/88 CCH */

#ifndef _ANSI_STDLIB_H
#define _ANSI_STDLIB_H

#include <standards.h>

#ifdef __STRICT_BSD__
#error This file should not be in a strictly BSD program
#endif

#ifdef __STRICT_ANSI__
#ifndef NULL
#define NULL	((void *)0)
#endif	/* NULL */

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long	size_t;
#endif	/* _SIZE_T */

#ifndef _WCHAR_T
#define _WCHAR_T
typedef unsigned short	wchar_t;
#endif	/* _WCHAR_T */

#else	/* !__STRICT_ANSI__ */
#include <stddef.h> /* get size_t, NULL, etc. */
#endif	/* !__STRICT_ANSI__ */

#ifndef ERANGE
/* Match definition in <sys/errno.h>. */
#define ERANGE 34		/* Result too large */
#endif	/* ERANGE */

#ifndef HUGE_VAL
/* Match definition in <math.h>. */
#define HUGE_VAL (1e999)
#endif	/* HUGE_VAL */

#if defined(__STRICT_ANSI__) || defined(_NEXT_SOURCE) 
#define EXIT_FAILURE (1)
#define EXIT_SUCCESS (0)
#define RAND_MAX (2147483647)
#define MB_CUR_MAX (1)

typedef struct {int quot, rem;} div_t;
typedef struct {long int quot, rem;} ldiv_t;

#ifdef __STDC__
extern double atof(const char *nptr);
#define atof(nptr) strtod(nptr, (char **)NULL)
extern int atoi(const char *nptr);
extern long int atol(const char *nptr);
extern double strtod(const char *nptr, char **endptr);
extern long int strtol(const char *nptr, char **endptr, int base);
extern unsigned long int strtoul(const char *nptr, char **endptr, int base);
extern int rand(void);
extern void srand(unsigned int seed);
extern void *calloc(size_t nmemb, size_t size);
extern void free(void *ptr);
extern void *malloc(size_t size);
extern void *realloc(void *ptr, size_t size);
#ifdef	__GNUC__
extern volatile void abort(void);
#else	/* !__GNUC__ */
extern void abort(void);
#endif	/* !__GNUC__ */
extern char *getenv(const char *name);
extern int system(const char *string);
extern void *bsearch(const void *key, const void *base,
	size_t nmemb, size_t size,
	int (*compar)(const void *, const void *));
#ifdef __STRICT_ANSI__
extern void qsort(void *base, size_t nmemb, size_t size,
	int (*compar)(const void *, const void *));
#else	/* !__STRICT_ANSI__ */
extern void *qsort(void *base, size_t nmemb, size_t size,
	int (*compar)(const void *, const void *));
#endif	/* !__STRICT_ANSI__ */
extern int abs(int j);
extern div_t div(int numer, int denom);
extern long int labs(long int j);
extern ldiv_t ldiv(long int numer, long int denom);
extern int mblen(const char *s, size_t n);
extern int mbtowc(wchar_t *pwc, const char *s, size_t n);
extern int wctomb(char *s, wchar_t wchar);
extern size_t mbstowcs(wchar_t *pwcs, const char *s, size_t n);
extern size_t wcstombs(char *s, const wchar_t *pwcs, size_t n);
#else	/* !__STDC__ */
extern double atof();
#define atof(nptr) strtod(nptr, (char **)NULL)
extern int atoi();
extern long int atol();
extern double strtod();
extern long int strtol();
extern unsigned long int strtoul();
extern int rand();
extern void srand();
extern void *calloc();
extern void free();
extern void *malloc();
extern void *realloc();
#ifdef	__GNUC__
extern volatile void abort()
#else	/* !__GNUC__ */
extern void abort();
#endif	/* !__GNUC__ */
extern char *getenv();
extern int system();
extern void *bsearch();
#ifdef __STRICT_ANSI__
extern void qsort();
#else	/* !__STRICT_ANSI__ */
extern void *qsort();
#endif	/* !__STRICT_ANSI__ */
extern int abs();
extern div_t div();
extern long int labs();
extern ldiv_t ldiv();
extern int mblen();
extern int mbtowc();
extern int wctomb();
extern size_t mbstowcs();
extern size_t wcstombs();
#endif /* !__STDC__ */
#endif /* __STRICT_ANSI__ || _NEXT_SOURCE */

#ifdef	__cplusplus
typedef void (*_cplus_fcn_void)(void);
extern int atexit(_cplus_fcn_void);
#else	/* !__cplusplus */
#ifdef __STDC__
extern int atexit(void (*fcn)(void));
#else	/* !__STDC__ */
extern int atexit();
#endif	/* !__STDC__ */
#endif	/* !__cplusplus */

#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
extern volatile void exit(int status);
#else	/* !__GNUC__ || __STRICT_ANSI__ */
#ifdef __STDC__
extern void exit(int status);
#else	/* !__STDC__ */
extern void exit();
#endif	/* !__STDC__ */
#endif	/* !__GNUC__ || __STRICT_ANSI__ */

#ifndef __STRICT_ANSI__
extern void *valloc(size_t size);
#if 0
extern void *alloca(size_t size);

#undef 	alloca
#define	alloca(x)	__builtin_alloca(x)
#endif

extern void cfree(void *ptr);
extern void vfree(void *ptr);
extern size_t malloc_size(void *ptr);
extern size_t malloc_good_size(size_t byteSize);
extern int malloc_debug(int level);

#ifdef	__cplusplus
typedef void (*_cplus_fcn_int)(int);
extern void (*malloc_error(_cplus_fcn_int))(int);
#else	/* !__cplusplus */
extern void (*malloc_error(void (*fcn)(int)))(int);
#endif	/* !__cplusplus */

extern size_t mstats(void);
#endif	/* !__STRICT_ANSI__ */

#endif	/* _ANSI_STDLIB_H */
