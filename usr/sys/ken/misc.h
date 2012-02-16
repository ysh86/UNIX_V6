/* asm */
void incupc(int pc, int *prof);
void display();
void spl1();
void spl5();
void spl7();

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

/* ken/subr.c */
void bcopy(void *from, void *to, int count);

/* ken/alloc.c */
struct filsys *getfs(int dev);
int badblock(struct filsys *afp, int abn, int dev);

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

/* ken/pipe.c */
void prele(struct inode *ip);

/* ken/sig.c */
char issig();
char psig();
