/* uintptr_t */
#include <stdint.h>


/* asm */
void incupc(int pc, int *prof);
void display();
void spl1();
void spl5();
void spl7();
int ldiv(int a, int b);
int lrem(int a, int b);
int fuibyte(char *a);
int fubyte(char *a);
int subyte(char *a, char b);
int fuiword(int *a);
void clearseg(int a);
char *copyin(char *a, char *b, int c);
char *copyout(char *a, char *b, int c);
void savu(int *a);
int lshift(unsigned int a[2], int b);
int dpcmp(int a, unsigned int b, unsigned int c, unsigned int d);
void dpadd(unsigned int a[2], int b);


/* dmr/bio.c */
struct buf *bread(int dev, int blkno);
struct buf *breada(int adev, int blkno, int rablkno);
void bwrite(struct buf *bp);
void bdwrite(struct buf *bp);
void bawrite(struct buf *bp);
void brelse(struct buf *bp);
struct buf *incore(int adev, int blkno);
struct buf *getblk(int dev, int blkno);
void iowait(struct buf *bp);
void notavail(struct buf *bp);
void iodone(struct buf *bp);
void clrbuf(struct buf *bp);
void binit();
void devstart(struct buf *bp, int *devloc, int devblk, int hbcom);
void rhstart(struct buf *bp, int *devloc, int devblk, int *abae);
void mapalloc(struct buf *abp);
void mapfree(struct buf *bp);
int swap(int blkno, char *coreaddr, int count, int rdflg);
void bflush(int dev);
void physio(int (*strat)(), struct buf *abp, int dev, int rw);
void geterror(struct buf *abp);

/* dmr/tty.c */
void cinit();


/* ken/alloc.c */
struct filsys;
void iinit();
struct buf *alloc(short dev);
void free(short dev, short bno);
short badblock(struct filsys *afp, unsigned short abn, short dev);
struct inode *ialloc(short dev);
void ifree(short dev, short ino);
struct filsys *getfs(short dev);

/* ken/prf.c */
void panic(char *s);
void printf(char fmt[],int x1,int x2,int x3,int x4,int x5,int x6,int x7,int x8,int x9,int xa,int xb,int xc);

/* ken/subr.c */
void bcopy(short *from, short *to, short count);
int bmap(struct inode *ip, int bn);
int passc(char c);
int cpass();

/* ken/slp.c */
struct proc;
void sleep(char *chan, char pri);
void wakeup(void *chan);
void setpri(struct proc *up);
int newproc();
void expand(int newsize);
void sched();
void swtch();

/* ken/prf.c */
void prdev(char *str, int dev);

/* ken/iget.c */
struct inode *iget(int dev, int ino);
void iput(struct inode *p);
void iupdat(struct inode *p);
void itrunc(int *ip);
void wdir(struct inode *ip);

/* ken/pipe.c */
void plock(struct inode *ip);
void prele(struct inode *ip);

/* ken/sig.c */
char issig();
char psig();

/* ken/fio.c */
short access(struct inode *aip, short mode);
short suser();

/* ken/nami.c */
struct inode *namei(int (*func)(), int flag);
int uchar();

/* ken/rdwri.c */
void readi(struct inode *aip);
void writei(struct inode *aip);
unsigned int max(unsigned int a, unsigned int b);
unsigned int min(unsigned int a, unsigned int b);

/* ken/malloc.c */
char *malloc(struct map *mp, int size);
void mfree(struct map *mp, int size, char *aa);

/* ken/main.c */
int estabur(int nt, int nd, int ns, int sep);
int nseg(int n);
