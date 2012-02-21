/* conf.h */
#define GET_MINOR(__data__) ((char)(((__data__)>>8)&0xFF))
#define GET_MAJOR(__data__) ((char)( (__data__)    &0xFF))
#define SET_MINOR(__dst__,__src__) ((__dst__) = ((__dst__)&0x00FF) | (((__src__)<<8)&0xFF00))
#define SET_MAJOR(__dst__,__src__) ((__dst__) = ((__dst__)&0xFF00) | ( (__src__)    &0x00FF))

/* param.h */
#define GET_LO(__data__) ((char)(((__data__)>>8)&0xFF))
#define GET_HI(__data__) ((char)( (__data__)    &0xFF))
#define SET_LO(__dst__,__src__) ((__dst__) = ((__dst__)&0x00FF) | (((__src__)<<8)&0xFF00))
#define SET_HI(__dst__,__src__) ((__dst__) = ((__dst__)&0xFF00) | ( (__src__)    &0x00FF))


/* asm */
void incupc(int pc, int *prof);
void display();
void spl1();
void spl5();
void spl7();
int ldiv(int a, int b);
int lrem(int a, int b);
int fuibyte(char a);
int fuiword(int *a);
void clearseg(int a);
void copyout(int *a, int b, int c);


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


/* ken/prf.c */
void panic(char *s);
void printf(char fmt[],int x1,int x2,int x3,int x4,int x5,int x6,int x7,int x8,int x9,int xa,int xb,int xc);

/* ken/subr.c */
void bcopy(void *from, void *to, int count);

/* ken/alloc.c */
struct filsys *getfs(int dev);
int badblock(struct filsys *afp, int abn, int dev);
struct inode *ialloc(int dev);
void ifree(int dev, int ino);
void free(int dev, int bno);
void iinit();

/* ken/slp.c */
void sleep(int chan, char pri);
void wakeup(int chan);
void setpri(struct proc *up);
int newproc();
void expand(int newsize);
void sched();

/* ken/prf.c */
void prdev(char *str, int dev);

/* ken/iget.c */
struct inode *iget(int dev, int ino);
void iput(struct inode *p);
void iupdat(struct inode *p, int *tm);
void itrunc(int *ip);
void wdir(struct inode *ip);

/* ken/pipe.c */
void prele(struct inode *ip);

/* ken/sig.c */
char issig();
char psig();

/* ken/fio.c */
void closei(struct inode *ip, char rw);
int suser();

/* ken/nami.c */
struct inode *namei(int (*func)(), int flag);
int uchar();

/* ken/rdwri.c */
void writei(struct inode *aip);
int min(int a, int b);

/* ken/malloc.c */
struct map
{
	char *m_size;
	char *m_addr;
};
void mfree(struct map *mp, int size, int aa);

/* ken/main.c */
int estabur(int nt, int nd, int ns, int sep);
int nseg(int n);
