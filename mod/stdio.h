/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)stdio.h	5.3 (Berkeley) 3/15/86
 */

#ifndef _ANSI_STDIO_H
#define _ANSI_STDIO_H

#include <standards.h>

#if defined(__STRICT_BSD__) || defined(__STRICT_ANSI__) 
	#ifndef NULL
		#define	NULL 	0 
	#endif 
#else
	#if defined (__STRICT_ANSI__)
		#ifndef NULL
			#define NULL	((void *)0)
		#endif
	#else
		#include <stddef.h> /* get NULL, errno */
		#include <stdarg.h> /* get va_list */
	#endif
#endif	/* __STRICT_BSD__ || __STRICT_ANSI__ */


#if defined(_NEXT_SOURCE) || defined(__STRICT_ANSI__)
	#define	BUFSIZ	1024
#endif /* _NEXT_SOURCE || __STRICT_ANSI__ */

extern struct _iobuf {
	int	_cnt;
	char	*_ptr;		/* should be unsigned char */
	char	*_base;		/* ditto */
	int	_bufsiz;
	short	_flag;
#ifdef	__NeXT__
	unsigned char	_file;	/* should be short */
#else
	char	_file;		/* should be short */
#endif	__NeXT__
	char	_smallbuf;	/* character for unbuf file */
} _iob[];

#define	_IOREAD	01
#define	_IOWRT	02
#if defined(__STRICT_ANSI__) || defined(_NEXT_SOURCE)
	#define	_IONBF	04
#endif /* __STRICT_ANSI__ || _NEXT_SOURCE */
#define	_IOMYBUF	010
#define	_IOEOF	020
#define	_IOERR	040
#define	_IOSTRG	0100
#define	_IOLBF	0200
#define	_IORW	0400

#if defined(__STRICT_ANSI__) || defined(_NEXT_SOURCE)
	#define	FILE	struct _iobuf
	#define	EOF	(-1)

	#define	stdin	(&_iob[0])
	#define	stdout	(&_iob[1])
	#define	stderr	(&_iob[2])
#endif /* __STRICT_ANSI__ || _NEXT_SOURCE */

#ifdef __STRICT_BSD__
	extern char *bsd_sprintf();
	extern char *bsd_vsprintf();
	#define sprintf bsd_sprintf
	#define vsprintf bsd_vsprintf
	extern FILE *fopen();
	extern FILE *freopen();
	extern long int ftell();
	extern char *fgets();
	extern char *gets();
#else	/* !__STRICT_BSD__ */
	#if defined(_NEXT_SOURCE) || defined(__STRICT_ANSI__)

		#ifndef _SIZE_T
			#define _SIZE_T
			typedef unsigned long	size_t;
		#endif  /* _SIZE_T */

		#ifndef _FPOS_T
			#define _FPOS_T
			typedef long fpos_t;
		#endif	/* _FPOS_T */

		#define _IOFBF	00	/* any value not equal to  */
					/* LBF or NBF */
		#define L_tmpnam 14	/* large enough to hold */
					/* tmpnam result */
		#define FOPEN_MAX 256	/* min files guaranteed */
					/* open simultaneously */
		#define FILENAME_MAX 1024  /* max len string that can */
					   /* be opened as file */
	
		#define SEEK_SET 0	/* arguments to fseek */
					/* function */
		#define SEEK_CUR 1
		#define SEEK_END 2
		#define TMP_MAX 25	/* min unique file names */
					/* from tmpnam */

		#if __STDC__
			extern int remove(const char *filename);
			extern int rename(const char *old_name, \
						const char *new_name);
			extern FILE *tmpfile(void);
			extern char *tmpnam(char *s);
			extern int fclose(FILE *stream);
			extern int fflush(FILE *stream);
			extern void setbuf(FILE *stream, char *buf);
			extern int setvbuf(FILE *stream, char *buf, \
						int mode, size_t size);
			extern int fprintf(FILE *stream, const char \
						*format, ...);
			extern int fscanf(FILE *stream, const char \
						*format, ...);
			extern int printf(const char *format, ...);
			extern int scanf(const char *format, ...);
			extern int sprintf(char *s, const char \
						*format, ...);
			extern int sscanf(const char *s, const \
						char *format, ...);
			#ifdef _VA_LIST
				extern int vfprintf(FILE *stream, \
					  const char *format, va_list arg);
				extern int vprintf(const char *format, \
						va_list arg);
				extern int vsprintf(char *s, const \
						char *format, va_list arg);
			#else
				extern int vfprintf();
				extern int vprintf();
				extern int vsprintf();
			#endif	/* _VA_LIST */

			extern int fgetc(FILE *stream);
			extern int fputc(int c, FILE *stream);
			extern int fputs(const char *s, FILE *stream);
			extern int getc(FILE *stream);
			extern int _flsbuf(unsigned char c, FILE *iop);
			extern int _filbuf(FILE *iop);
			extern int getchar(void);
			extern int putc(int c, FILE *stream);
			extern int putchar(int c);
			extern int puts(const char *s);
			extern int ungetc(int c, FILE *stream);
			extern size_t fread(void *ptr, size_t size, \
						size_t nmemb, FILE *stream);
			extern size_t fwrite(const void *ptr, size_t \
					size, size_t nmemb, FILE *stream);
			extern int fgetpos(FILE *stream, fpos_t *pos);
			extern int fseek(FILE *stream, long int \
						offset, int whence);
			extern int fsetpos(FILE *stream, const \
						fpos_t *pos);
			extern void rewind(FILE *stream);
			extern void clearerr(FILE *stream);
			extern int feof(FILE *stream);
			extern int ferror(FILE *stream);
			extern void perror(const char *s);
			extern FILE *fopen(const char *filename, \
						const char *mode);
			extern FILE *freopen(const char *filename, \
						const char *mode, FILE *stream);
			extern long int ftell(FILE *stream);
			extern char *fgets(char *s, int n, FILE \
						*stream);
			extern char *gets(char *s);
		#else
			extern int remove();
			extern int rename();
			extern FILE *tmpfile();
			extern char *tmpnam();
			extern int fclose();
			extern int fflush();
			extern void setbuf();
			extern int setvbuf();
			extern int fprintf();
			extern int fscanf();
			extern int printf();
			extern int scanf();
			extern int sprintf();
			extern int sscanf();
			extern int vfprintf();
			extern int vprintf();
			extern int vsprintf();
			extern int fgetc();
			extern int fputc();
			extern int fputs();
			extern int getc();
			extern int _flsbuf();
			extern int _filbuf();
			extern int getchar();
			extern int putc();
			extern int putchar();
			extern int puts();
			extern int ungetc();
			extern size_t fread();
			extern size_t fwrite();
			extern int fgetpos();
			extern int fseek();
			extern int fsetpos();
			extern void rewind();
			extern void clearerr();
			extern int feof();
			extern int ferror();
			extern void perror();
			extern FILE *fopen();
			extern FILE *freopen();
			extern long int ftell();
			extern char *fgets();
			extern char *gets();
		#endif /* __STDC__ */
	#endif /* _NEXT_SOURCE || __STRICT_ANSI__ */
#endif /* __STRICT_BSD__ */	

#ifdef _POSIX_SOURCE
	#define L_ctermid	256
	#if __STDC__
		extern char *ctermid(char *s);
	#else
		extern char *ctermid();
	#endif /* __STDC__ */
#endif /* _POSIX_SOURCE */

#if defined(_POSIX_SOURCE) || !defined(__STRICT_ANSI__)
#if 0
	#ifdef __STDC__
		extern int fileno(FILE *stream);
	#else
		extern int fileno();
	#endif /* __STDC__ */
#endif
	#define	fileno(p)	((p)->_file)
#endif /* _POSIX_SOURCE && !__STRICT_ANSI__ */

#if defined(__STRICT_ANSI__) || defined(_NEXT_SOURCE)
	#ifndef lint
		#define	getc(p)	(--(p)->_cnt>=0? \
			(int)(*(unsigned char *)(p)->_ptr++):_filbuf(p))
		#endif /* not lint */

		#define	getchar()	getc(stdin)

		#ifndef lint
			#define putc(x, p)	(--(p)->_cnt >= 0 ?\
			(int)(*(unsigned char *)(p)->_ptr++ = (x)) :\
			(((p)->_flag & _IOLBF) && -(p)->_cnt < (p)->_bufsiz ?\
				((*(p)->_ptr = (x)) != '\n' ?\
				(int)(*(unsigned char *)(p)->_ptr++) :\
				_flsbuf(*(unsigned char *)(p)->_ptr, p)) :\
				_flsbuf((unsigned char)(x), p)))
		#endif /* not lint */
		#define	putchar(x)	putc(x,stdout)
		#define	feof(p)		(((p)->_flag&_IOEOF)!=0)
		#define	ferror(p)	(((p)->_flag&_IOERR)!=0)
		#define	clearerr(p)	((p)->_flag &= ~(_IOERR|_IOEOF))
#endif	/* __STRICT_ANSI__ || _NEXT_SOURCE */
		
#if	defined(__STRICT_BSD__)
		extern FILE 	*fdopen();
		extern FILE 	*popen();
		extern int	pclose();
#else
		#if !defined(__STRICT_ANSI__) || defined(_POSIX_SOURCE) \
			|| defined(_NEXT_SOURCE)
			#ifdef __STDC__
				extern FILE *fdopen(int fildes, \
					const char *mode);
			#else
				extern FILE *fdopen();
			#endif	/* __STDC__ */
		#endif /* !__STRICT_ANSI__ || _POSIX_SOURCE || _NEXT_SOURCE */

		#if defined(_NEXT_SOURCE)
			#ifdef __STDC__
				extern FILE *popen(const char *command, \
						   const char *mode);
				extern int pclose(FILE *stream);
			#else
				extern FILE *popen();
				extern int pclose();
			#endif	/* __STDC__ */
		#endif /* _NEXT_SOURCE */
#endif /* __STRICT_BSD__ */



#endif /* _ANSI_STDIO_H */
