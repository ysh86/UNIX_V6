/* asm */
void incupc(int pc, int *prof);
void display();
void spl1();
void spl5();
void spl7();
int ldiv(int a, int b);
int lrem(int a, int b);

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

/* ken/slp.c */
void sleep(int chan, char pri);
void wakeup(int chan);
void setpri(struct proc *up);

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
