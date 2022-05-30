#include "../param.h"
#include "../user.h"
#include "../userx.h"
#include "../filsys.h"
#include "../file.h"
#include "../filex.h"
#include "../conf.h"
#include "../inode.h"
#include "../inodex.h"
#include "../reg.h"
#include "../systm.h"

#include "../declarations.h"

/*
 * Convert a user supplied
 * file descriptor into a pointer
 * to a file structure.
 * Only task is to check range
 * of the descriptor.
 */
struct file *getf(short f)
{
	register struct file *fp;
	register short rf;

	rf = f;
	if (0<=rf && rf<NOFILE) {
		fp = u.u_ofile[rf];
		if(fp != NULL)
			return(fp);
	}
	u.u_error = EBADF;
	return(NULL);
}

/*
 * Internal form of close.
 * Decrement reference count on
 * file structure.
 * Also make sure the pipe protocol
 * does not constipate.
 *
 * Decrement reference count on the inode following
 * removal to the referencing file structure.
 * On the last close switch out the the device handler for
 * special files.  Note that the handler is called
 * on every open but only the last close.
 */
void closef(struct file *fp)
{
	register struct inode *ip;
	register struct file *rfp;
	char flag;
	register int (*cfunc)();

	if ((rfp = fp) == NULL)
		return;
	ip = rfp->f_inode;
	if(rfp->f_flag&FPIPE) {
		ip->i_mode &= ~(IREAD|IWRITE);
		wakeup(ip+1);
		wakeup(ip+2);
	}
	if (--rfp->f_count > 0)
		return;
	plock(ip);
	iput(ip);
	if ((ip->i_mode&IFMT) == IFCHR)
		cfunc = cdevsw[GET_D_MAJOR(ip->i_addr[0])].d_close;
	else if ((ip->i_mode&IFMT)== IFBLK)
		cfunc = bdevsw[GET_D_MAJOR(ip->i_addr[0])].d_close;
	else
		return;
	flag = rfp->f_flag&FWRITE;
	for (rfp=file; rfp < &file[NFILE]; rfp++)
		if (rfp->f_count && rfp->f_inode==ip)
			return;
	(*cfunc)(ip->i_addr[0], flag);
}

/*
 * openi called to allow handler
 * of special files to initialize and
 * validate before actual IO.
 * Called on all sorts of opens
 * and also on mount.
 */
void openi(struct inode *ip, short rw)
{
	register struct inode *rip;
	register short dev, maj;

	rip = ip;
	dev = rip->i_addr[0];
	maj = GET_D_MAJOR(rip->i_addr[0]) & 0377;
	switch(rip->i_mode&IFMT) {

	case IFCHR:
		if(maj >= nchrdev)
			goto bad;
		(*cdevsw[maj].d_open)(dev, rw);
		break;

	case IFBLK:
		if(maj >= nblkdev)
			goto bad;
		(*bdevsw[maj].d_open)(dev, rw);
	}
	return;

bad:
	u.u_error = ENXIO;
}

/*
 * Check mode permission on inode pointer.
 * Mode is READ, WRITE or EXEC.
 * In the case of WRITE, the
 * read-only status of the file
 * system is checked.
 * Also in WRITE, prototype text
 * segments cannot be written.
 * The mode is shifted to select
 * the owner/group/other fields.
 * The super user is granted all
 * permissions.
 */
short access(struct inode *aip, short mode)
{
	register struct inode *ip;
	register short m;

	ip = aip;
	m = mode;
	if(m == IWRITE) {
		if(getfs(ip->i_dev)->s_ronly != 0) {
			u.u_error = EROFS;
			return(1);
		}
		if(ip->i_flag & ITEXT) {
			u.u_error = ETXTBSY;
			return(1);
		}
	}
	if(u.u_uid == 0)
		return(0);
	if(u.u_uid != ip->i_uid) {
		m >>= 3;
		if(u.u_gid != ip->i_gid)
			m >>= 3;
	}
	if((ip->i_mode&m) != 0)
		return(0);

/* bad: */
	u.u_error = EACCES;
	return(1);
}

/*
 * Look up a pathname and test if
 * the resultant inode is owned by the
 * current user.
 * If not, try for super-user.
 * If permission is granted,
 * return inode pointer.
 */
struct inode *owner()
{
	register struct inode *ip;

	if ((ip = namei(uchar, 0)) == NULL)
		return(NULL);
	if(u.u_uid == ip->i_uid)
		return(ip);
	if (suser())
		return(ip);
	iput(ip);
	return(NULL);
}

/*
 * Test if the current user is the
 * super user.
 */
short suser()
{

	if(u.u_uid == 0)
		return(1);
	u.u_error = EPERM;
	return(0);
}

/*
 * Allocate a user file descriptor.
 */
short ufalloc()
{
	register short i;

	for (i=0; i<NOFILE; i++)
		if (u.u_ofile[i] == NULL) {
			u.u_ar0[R0] = i;
			return(i);
		}
	u.u_error = EMFILE;
	return(-1);
}

/*
 * Allocate a user file descriptor
 * and a file structure.
 * Initialize the descriptor
 * to point at the file structure.
 *
 * no file -- if there are no available
 * 	file structures.
 */
struct file *falloc()
{
	register struct file *fp;
	register short i;

	if ((i = ufalloc()) < 0)
		return(NULL);
	for (fp = &file[0]; fp < &file[NFILE]; fp++)
		if (fp->f_count==0) {
			u.u_ofile[i] = fp;
			fp->f_count++;
			fp->f_offset[0] = 0;
			fp->f_offset[1] = 0;
			return(fp);
		}
	printf("no file\n",0,0,0,0,0,0,0,0,0,0,0,0);
	u.u_error = ENFILE;
	return(NULL);
}
