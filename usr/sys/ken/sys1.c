#
#include "../param.h"
#include "../systm.h"
#include "../user.h"
#include "../userx.h"
#include "../proc.h"
#include "../procx.h"
#include "../buf.h"
#include "../bufx.h"
#include "../reg.h"
#include "../inode.h"
#include "../inodex.h"
#include "../seg.h"

/*
 * exec system call.
 */
exec()
{
	register nc;
	register char *cp;
	register struct buf *bp;
	int na, bno, ucp, ap, c;
	struct inode *ip;
	extern int uchar();

	if ((ip = namei(uchar, 0)) == NULL)
		return;
	bno = 0;
	bp = 0;
	if(access(ip, IEXEC))
		goto bad;
	if((ip->i_mode & IFMT) != 0 ||
	   (ip->i_mode & (IEXEC|(IEXEC>>3)|(IEXEC>>6))) == 0) {
		u.u_error = EACCES;
		goto bad;
	}
	/*
	 * Collect arguments on "file" in swap space.
	 */
	na = 0;
	nc = 0;
	if ((bno = malloc(swapmap,(NCARGS+511)/512)) == 0)
		panic("Out of swap");
	if (u.u_arg[1]) while(ap = fuword(u.u_arg[1])) {
		na++;
		if(ap == -1)
			u.u_error = EFAULT;
		u.u_arg[1] =+ 2;
		do {
			if (nc >= NCARGS-1)
				u.u_error = E2BIG;
			if ((c = fubyte(ap++)) < 0)
				u.u_error = EFAULT;
			if (u.u_error)
				goto bad;
			if ((nc&0777) == 0) {
				if (bp)
					bawrite(bp);
				bp = getblk(swapdev, bno+(nc>>9));
				cp = bp->b_addr;
			}
			nc++;
			*cp++ = c;
		} while (c>0);
	}
	if (bp)
		bawrite(bp);
	bp = 0;
	if((nc&1) != 0)
		nc++;
	if (getxfile(ip, nc) || u.u_error)
		goto bad;

	/*
	 * copy back arglist
	 */

	ucp = -nc;
	ap = ucp - na*2 - 4;
	u.u_ar0[R6] = ap;
	suword(ap, na);
	nc = 0;
	while(na--) {
		suword(ap=+2, ucp);
		do {
			if ((nc&0777) == 0) {
				if (bp)
					brelse(bp);
				bp = bread(swapdev, bno+(nc>>9));
				cp = bp->b_addr;
			}
			subyte(ucp++, (c = *cp++));
			nc++;
		} while(c&0377);
	}
	suword(ap+2, -1);
	setregs();
bad:
	if (bp)
		brelse(bp);
	if(bno)
		mfree(swapmap, (NCARGS+511)/512, bno);
	iput(ip);
}

/*
 * Read in and set up memory for executed file.
 * Zero return is normal;
 * non-zero means only the text is being replaced
 */
getxfile(aip, nargc)
{
	register struct inode *ip;
	register ds, sep;
	int ts, ss, overlay, i;

	/*
	 * read in first few bytes
	 * of file for segment
	 * sizes:
	 * ux_mag = 407/410/411/405
	 *  407 is plain executable
	 *  410 is RO text
	 *  411 is separated ID
	 *  405 is overlaid text
	 */

	ip = aip;
	u.u_base = &u.u_exdata;
	u.u_count = sizeof(u.u_exdata);
	u.u_offset[1] = 0;
	u.u_offset[0] = 0;
	u.u_segflg = 1;
	readi(ip);
	u.u_segflg = 0;
	if(u.u_error)
		goto bad;
	if (u.u_count!=0) {
		u.u_error = ENOEXEC;
		goto bad;
	}
	sep = 0;
	overlay = 0;
	if(u.u_exdata.ux_mag == 0407) {
		u.u_exdata.ux_dsize =+ u.u_exdata.ux_tsize;
		u.u_exdata.ux_tsize = 0;
	} else if (u.u_exdata.ux_mag == 0411)
		sep++;
	else if (u.u_exdata.ux_mag == 0405)
		overlay++;
	else if (u.u_exdata.ux_mag != 0410) {
		u.u_error = ENOEXEC;
		goto bad;
	}
	if(u.u_exdata.ux_tsize!=0 && (ip->i_flag&ITEXT)==0 && ip->i_count!=1) {
		u.u_error = ETXTBSY;
		goto bad;
	}

	/*
	 * find text and data sizes
	 * try them out for possible
	 * exceed of max sizes
	 */

	ts = ((u.u_exdata.ux_tsize+63)>>6) & 01777;
	ds = ((u.u_exdata.ux_dsize+u.u_exdata.ux_bsize+63)>>6) & 01777;
	ss = SSIZE + (nargc>>6);
	if (overlay) {
		if (u.u_sep==0 && ctos(ts) != ctos(u.u_tsize) || nargc) {
			u.u_error = ENOMEM;
			goto bad;
		}
		ds = u.u_dsize;
		ss = u.u_ssize;
		sep = u.u_sep;
		xfree();
		xalloc(ip);
		u.u_ar0[R7] = u.u_exdata.ux_entloc & ~01;
	} else {
		if(estabur(ts, ds, ss, sep, RO))
			goto bad;
	
		/*
		 * allocate and clear core
		 * at this point, committed
		 * to the new image
		 */
	
		u.u_prof[3] = 0;
		xfree();
		i = USIZE+ds+ss;
		expand(i);
		while(--i >= USIZE)
			clearseg(u.u_procp->p_addr+i);
		xalloc(ip);
	
		/*
		 * read in data segment
		 */
	
		estabur(0, ds, 0, 0, RO);
		u.u_base = 0;
		u.u_offset[1] = 020+u.u_exdata.ux_tsize;
		u.u_count = u.u_exdata.ux_dsize;
		readi(ip);
		/*
		 * set SUID/SGID protections, if no tracing
		 */
		if ((u.u_procp->p_flag&STRC)==0) {
			if(ip->i_mode&ISUID)
				if(u.u_uid != 0) {
					u.u_uid = ip->i_uid;
					u.u_procp->p_uid = ip->i_uid;
				}
			if(ip->i_mode&ISGID)
				u.u_gid = ip->i_gid;
		}
	}
	u.u_tsize = ts;
	u.u_dsize = ds;
	u.u_ssize = ss;
	u.u_sep = sep;
	estabur(ts, ds, ss, sep, RO);
bad:
	return(overlay);
}

/*
 * Clear registers on exec
 */
setregs()
{
	register int *rp;
	register char *cp;
	register i;

	for(rp = &u.u_signal[0]; rp < &u.u_signal[NSIG]; rp++)
		if((*rp & 1) == 0)
			*rp = 0;
	for(cp = &regloc[0]; cp < &regloc[6];)
		u.u_ar0[*cp++] = 0;
	u.u_ar0[R7] = u.u_exdata.ux_entloc & ~01;
	for(rp = &u.u_fsav[0]; rp < &u.u_fsav[25];)
		*rp++ = 0;
	for (rp = &u.u_ofile[FCLFILE]; rp < &u.u_ofile[NOFILE];) {
		closef(*rp);
		*rp++ = NULL;
	}
	/*
	 * Remember file name for accounting.
	 */
	u.u_acflag =& ~1;
	for (i=0; i<DIRSIZ; i++)
		u.u_comm[i] = u.u_dbuf[i];
}

/*
 * exit system call:
 * pass back caller's r0
 */
rexit()
{

	u.u_arg[0] = u.u_ar0[R0] << 8;
	exit();
}

/*
 * Release resources.
 * Save u. area for parent to look at.
 * Enter zombie state.
 * Wake up parent and init processes,
 * and dispose of children.
 */
exit()
{
	register int *a;
	register struct proc *p, *q;

	p = u.u_procp;
	p->p_flag =& ~STRC;
	p->p_clktim = 0;
	for(a = &u.u_signal[0]; a < &u.u_signal[NSIG];)
		*a++ = 1;
	for(a = &u.u_ofile[0]; a < &u.u_ofile[NOFILE]; a++)
		if (*a) {
			closef(*a);
			*a = NULL;
		}
	plock(u.u_cdir);
	iput(u.u_cdir);
	xfree();
	acct();
	mfree(coremap, p->p_size, p->p_addr);
	p->p_stat = SZOMB;
	p->xp_xstat = u.u_arg[0];
	p->xp_utime = u.u_cutime + u.u_utime;
	p->xp_stime = u.u_cstime + u.u_stime;
	for(q = &proc[0]; q < &proc[NPROC]; q++)
		if(q->p_ppid == p->p_pid) {
			wakeup(&proc[1]);
			q->p_ppid = 1;
			if (q->p_stat==SSTOP)
				setrun(q);
		}
	for(q = &proc[0]; q < &proc[NPROC]; q++)
		if(p->p_ppid == q->p_pid) {
			wakeup(q);
			swtch();
			/* no return */
		}
	printf("Init proc dead");
	swtch();
}

/*
 * Wait system call.
 * Search for a terminated (zombie) child,
 * finally lay it to rest, and collect its status.
 * Look also for stopped (traced) children,
 * and pass back status from them.
 */
wait()
{
	register f;
	register struct proc *p;

	f = 0;

loop:
	for(p = &proc[0]; p < &proc[NPROC]; p++)
	if(p->p_ppid == u.u_procp->p_pid) {
		f++;
		if(p->p_stat == SZOMB) {
			u.u_ar0[R0] = p->p_pid;
			u.u_ar0[R1] = p->xp_xstat;
			u.u_cutime =+ p->xp_utime;
			u.u_cstime =+ p->xp_stime;
			p->p_stat = NULL;
			p->p_pid = 0;
			p->p_ppid = 0;
			p->p_sig = 0;
			p->p_pgrp = 0;
			p->p_flag = 0;
			p->p_wchan = 0;
			return;
		}
		if(p->p_stat == SSTOP) {
			if((p->p_flag&SWTED) == 0) {
				p->p_flag =| SWTED;
				u.u_ar0[R0] = p->p_pid;
				u.u_ar0[R1] = (fsig(p)<<8) | 0177;
				return;
			}
			continue;
		}
	}
	if(f) {
		sleep(u.u_procp, PWAIT);
		goto loop;
	}
	u.u_error = ECHILD;
}

/*
 * fork system call.
 */
fork()
{
	register struct proc *p1, *p2;
	register a;

	/*
	 * Make sure there's enough swap space for max
	 * core image, thus reducing chances of running out
	 */
	if ((a = malloc(swapmap, ctob(MAXMEM))) == 0) {
		u.u_error = ENOMEM;
		goto out;
	}
	mfree(swapmap, ctob(MAXMEM), a);
	p1 = u.u_procp;
	for(p2 = &proc[0]; p2 < &proc[NPROC]; p2++)
		if(p2->p_stat == NULL)
			goto found;
	u.u_error = EAGAIN;
	goto out;

found:
	if(newproc()) {
		u.u_ar0[R0] = p1->p_pid;
		u.u_start = time;
		u.u_cstime = 0;
		u.u_stime = 0;
		u.u_cutime = 0;
		u.u_utime = 0;
		u.u_acflag =| 1;
		return;
	}
	u.u_ar0[R0] = p2->p_pid;

out:
	u.u_ar0[R7] =+ 2;
}

/*
 * break system call.
 *  -- bad planning: "break" is a dirty word in C.
 */
sbreak()
{
	register a, n, d;
	int i;

	/*
	 * set n to new data size
	 * set d to new-old
	 * set n to new total size
	 */

	n = (((u.u_arg[0]+63)>>6) & 01777);
	if(!u.u_sep)
		n =- ctos(u.u_tsize) * 128;
	if(n < 0)
		n = 0;
	d = n - u.u_dsize;
	n =+ USIZE+u.u_ssize;
	if(estabur(u.u_tsize, u.u_dsize+d, u.u_ssize, u.u_sep, RO))
		return;
	u.u_dsize =+ d;
	if(d > 0)
		goto bigger;
	a = u.u_procp->p_addr + n - u.u_ssize;
	i = n;
	n = u.u_ssize;
	while(n--) {
		copyseg(a-d, a);
		a++;
	}
	expand(i);
	return;

bigger:
	expand(n);
	a = u.u_procp->p_addr + n;
	n = u.u_ssize;
	while(n--) {
		a--;
		copyseg(a-d, a);
	}
	while(d--)
		clearseg(--a);
}
