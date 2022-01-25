#
#include "../param.h"
#include "../systm.h"
#include "../user.h"
#include "../userx.h"
#include "../reg.h"
#include "../file.h"
#include "../filex.h"
#include "../inode.h"
#include "../inodex.h"

/*
 * read system call
 */
read()
{
	rdwr(FREAD);
}

/*
 * write system call
 */
write()
{
	rdwr(FWRITE);
}

/*
 * common code for read and write calls:
 * check permissions, set base, count, and offset,
 * and switch out to readi, writei, or pipe code.
 */
rdwr(mode)
{
	register *fp, *ip, m;

	m = mode;
	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
	if((fp->f_flag&m) == 0) {
		u.u_error = EBADF;
		return;
	}
	u.u_base = u.u_arg[0];
	u.u_count = u.u_arg[1];
	u.u_segflg = 0;
	if(fp->f_flag&FPIPE) {
		if(m==FREAD)
			readp(fp); else
			writep(fp);
	} else {
		ip = fp->f_inode;
		u.u_offset[1] = fp->f_offset[1];
		u.u_offset[0] = fp->f_offset[0];
		if((ip->i_mode&(IFCHR&IFBLK)) == 0)
			plock(ip);
		if(m==FREAD)
			readi(ip); else
			writei(ip);
		if((ip->i_mode&(IFCHR&IFBLK)) == 0)
			prele(ip);
		dpadd(fp->f_offset, u.u_arg[1]-u.u_count);
	}
	u.u_ar0[R0] = u.u_arg[1]-u.u_count;
}

/*
 * open system call
 */
open()
{
	register *ip;
	extern uchar;

	ip = namei(&uchar, 0);
	if(ip == NULL)
		return;
	u.u_arg[1]++;
	open1(ip, u.u_arg[1], 0);
}

/*
 * creat system call
 */
creat()
{
	register *ip;
	extern uchar;

	ip = namei(&uchar, 1);
	if(ip == NULL) {
		if(u.u_error)
			return;
		ip = maknode(u.u_arg[1]&07777&(~ISVTX));
		if (ip==NULL)
			return;
		open1(ip, FWRITE, 2);
	} else
		open1(ip, FWRITE, 1);
}

/*
 * common code for open and creat.
 * Check permissions, allocate an open file structure,
 * and call the device open routine if any.
 */
open1(ip, mode, trf)
int *ip;
{
	register struct file *fp;
	register *rip, m;
	int i;

	rip = ip;
	m = mode;
	if(trf != 2) {
		if(m&FREAD)
			access(rip, IREAD);
		if(m&FWRITE) {
			access(rip, IWRITE);
			if((rip->i_mode&IFMT) == IFDIR)
				u.u_error = EISDIR;
		}
	}
	if(u.u_error)
		goto out;
	if(trf == 1)
		itrunc(rip);
	prele(rip);
	if ((fp = falloc()) == NULL)
		goto out;
	fp->f_flag = m&(FREAD|FWRITE);
	fp->f_inode = rip;
	i = u.u_ar0[R0];
	openi(rip, m&FWRITE);
	if(u.u_error == 0)
		return;
	u.u_ofile[i] = NULL;
	fp->f_count--;

out:
	iput(rip);
}

/*
 * close system call
 */
close()
{
	register *fp;

	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
	u.u_ofile[u.u_ar0[R0]] = NULL;
	closef(fp);
}

/*
 * seek system call
 */
seek()
{
	int n[2];
	register *fp, t;

	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
	if(fp->f_flag&FPIPE) {
		u.u_error = ESPIPE;
		return;
	}
	t = u.u_arg[1];
	if(t > 2) {
		n[1] = u.u_arg[0]<<9;
		n[0] = u.u_arg[0]>>7;
		if(t == 3)
			n[0] =& 0777;
	} else {
		n[1] = u.u_arg[0];
		n[0] = 0;
		if(t!=0 && n[1]<0)
			n[0] = -1;
	}
	switch(t) {

	case 1:
	case 4:
		n[0] =+ fp->f_offset[0];
		dpadd(n, fp->f_offset[1]);
		break;

	default:
		n[0] =+ fp->f_inode->i_size0&0377;
		dpadd(n, fp->f_inode->i_size1);

	case 0:
	case 3:
		;
	}
	if (n[0] & ~0777)
		u.u_error = EFBIG;
	else {
		fp->f_offset[1] = n[1];
		fp->f_offset[0] = n[0];
	}
}

/*
 * Tell -- discover offset of file R/W pointer
 */
tell()
{
	register struct file *fp;

	if (fp = getf(u.u_ar0[R0])) {
		u.u_ar0[R0] = fp->f_offset[0];
		u.u_ar0[R1] = fp->f_offset[1];
	}
}

/*
 * link system call
 */
link()
{
	register *ip, *xp;
	extern uchar;

	ip = namei(&uchar, 0);
	if(ip == NULL)
		return;
	if(ip->i_nlink >= 127) {
		u.u_error = EMLINK;
		goto out;
	}
	if((ip->i_mode&IFMT)==IFDIR && !suser())
		goto out;
	/*
	 * Unlock to avoid possibly hanging the namei.
	 * Sadly, this means races. (Suppose someone
	 * deletes the file in the meantime?)
	 * Nor can it be locked again later
	 * because then there will be deadly
	 * embraces.
	 */
	prele(ip);
	u.u_dirp = u.u_arg[1];
	xp = namei(&uchar, 1);
	if(xp != NULL) {
		u.u_error = EEXIST;
		iput(xp);
	}
	if(u.u_error)
		goto out;
	if(u.u_pdir->i_dev != ip->i_dev) {
		iput(u.u_pdir);
		u.u_error = EXDEV;
		goto out;
	}
	wdir(ip);
	ip->i_nlink++;
	ip->i_flag =| IUPD;

out:
	iput(ip);
}

/*
 * mknod system call
 */
mknod()
{
	register *ip;
	extern uchar;

	if(suser()) {
		ip = namei(&uchar, 1);
		if(ip != NULL) {
			u.u_error = EEXIST;
			goto out;
		}
	}
	if(u.u_error)
		return;
	ip = maknode(u.u_arg[1]);
	if (ip==NULL)
		return;
	ip->i_addr[0] = u.u_arg[2];

out:
	iput(ip);
}

/*
 * sleep system call
 * not to be confused with the sleep internal routine.
 */
sslep()
{
	long d;

	spl7();
	d = time + u.u_ar0[R0];
	while (time < d) {
		if (tout<=time || d<tout)
			tout = d;
		sleep(&tout, PSLEP);
	}
	spl0();
}

/*
 * access system call
 */
saccess()
{
	extern uchar;
	register svuid, svgid;
	register *ip;

	svuid = u.u_uid;
	svgid = u.u_gid;
	u.u_uid = u.u_ruid;
	u.u_gid = u.u_rgid;
	ip = namei(&uchar, 0);
	if (ip != NULL) {
		if (u.u_arg[1]&(IREAD>>6))
			access(ip, IREAD);
		if (u.u_arg[1]&(IWRITE>>6))
			access(ip, IWRITE);
		if (u.u_arg[1]&(IEXEC>>6))
			access(ip, IEXEC);
		iput(ip);
	}
	u.u_uid = svuid;
	u.u_gid = svgid;
}
