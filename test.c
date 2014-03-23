extern	enum foobar {
	fb_x,
	fb_y = 2,
	fb_z
} foobar_i, *foobar_ip;

/* #include <stdio.h> */

typedef struct mystruct {
	int	x, *y, z;
	double	(*func)(void);
	struct {
	   int sx, sy, sz;
	} substruct;
	int	bits : 4;
	int	a[5][3];
	struct mystruct *next;
} mystruct;

typedef mystruct yourstruct;

extern
struct {
	int	a;
	char*	b;
	int	bits : 4;
	int	d[5][3];
} unamed_struct;


typedef void *ESO;

extern char c;
extern unsigned char C;
extern short s;
extern unsigned short S;
extern int i;
extern unsigned int I;
extern long l;
extern unsigned long L;
extern float f;
extern double d;
extern long double D;

extern double* x;
extern double** y;
extern int ia[];
extern int ja[5];
extern int ka[][3];

void	foo(int hello, double x, char* y, ESO _HOLD_x, ESO _HOLD_REST_args);
void	bar(unsigned, ESO x, ESO y);
ESO	baz(ESO foo, double d, ESO bar);
/* extern	void (*fp)(void); */

void    func_fbd(int foo, char bar, double d);
void    func_no_void();
void    func_void(void);

union ux {
	char	c;
	int	i;
	double	d;
	char   *s;
	enum foobar	f;
};

struct sy {
	int	a;
	double b;
	struct sy *xp;
};

