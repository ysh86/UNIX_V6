#
#include "../param.h"
#include "../systm.h"
#include "../user.h"
#include "../userx.h"
#include "../proc.h"
#include "../text.h"
#include "../textx.h"
#include "../inode.h"
#include "../inodex.h"
#include "../buf.h"
#include "../bufx.h"
#include "../seg.h"

/*
 * Swap out process p.
 * The ff flag causes its core to be freed--
 * it may be off when called to create an image for a
 * child process in newproc.
 * Os is the old size of the data area of the process,
 * and is supplied during core expansion swaps.
 *
 * panic: out of swap space
 */
xswap(p, ff, os)
int *p;
{
	register *rp, a;

	rp = p;
	if(os == 0)
		os = rp->p_size;
	a = malloc(swapmap, ctob(rp->p_size));
	if(a == NULL)
		panic("out of swap space");
	rp->p_flag =| SLOCK;
	xccdec(rp->p_textp);
	swap(a, rp->p_addr, os, B_WRITE);
	if(ff)
		mfree(coremap, os, rp->p_addr);
	rp->p_addr = a;
	rp->p_flag =& ~(SLOAD|SLOCK);
	rp->p_time = 0;
	if(runout) {
		runout = 0;
		wakeup(&runout);
	}
}

/*
 * relinquish use of the shared text segment
 * of a process.
 */
xfree()
{
	register struct text *xp;
	register struct inode *ip;

	if((xp=u.u_procp->p_textp) == NULL)
		return;
	xlock(xp);
	xp->x_flag =& ~XLOCK;
	ip = xp->x_iptr;
	if(--xp->x_count==0 && (ip->i_mode&ISVTX)==0) {
		xp->x_iptr = NULL;
		mfree(swapmap, ctob(xp->x_size), xp->x_daddr);
		mfree(coremap, xp->x_size, xp->x_caddr);
		ip->i_flag =& ~ITEXT;
		if (ip->i_flag&ILOCK)
			ip->i_count--;
		else
			iput(ip);
	} else
		xccdec(xp);
	u.u_procp->p_textp = NULL;
}

/*
 * Attach to a shared text segment.
 * If there is no shared text, just return.
 * If there is, hook up to it:
 * if it is not currently being used, it has to be read
 * in from the inode (ip); the written bit is set to force it
 * to be written out as appropriate.
 * If it is being used, but is not currently in core,
 * a swap has to be done to get it back.
 */
xalloc(ip)
int *ip;
{
	register struct text *xp;
	register *rp, ts;

	if(u.u_exdata.ux_tsize == 0)
		return;
	rp = NULL;
	for (xp = &text[0]; xp < &text[NTEXT]; xp++) {
		if(xp->x_iptr == NULL) {
			if(rp == NULL)
				rp = xp;
			continue;
		}
		if(xp->x_iptr == ip) {
			xlock(xp);
			xp->x_count++;
			u.u_procp->p_textp = xp;
			if (xp->x_ccount == 0)
				xexpand(xp);
			else
				xp->x_ccount++;
			xunlock(xp);
			return;
		}
	}
	if((xp=rp) == NULL) {
		printf("out of text");
		psignal(u.u_procp, SIGKIL);
		return;
	}
	xp->x_flag = XLOAD|XLOCK;
	xp->x_count = 1;
	xp->x_ccount = 0;
	xp->x_iptr = rp = ip;
	rp->i_flag =| ITEXT;
	rp->i_count++;
	ts = ((u.u_exdata.ux_tsize+63)>>6) & 01777;
	xp->x_size = ts;
	if((xp->x_daddr = malloc(swapmap, ctob(ts))) == NULL)
		panic("out of swap space");
	u.u_procp->p_textp = xp;
	xexpand(xp);
	estabur(ts, 0, 0, 0, RW);
	u.u_count = u.u_exdata.ux_tsize;
	u.u_offset[0] = 0;
	u.u_offset[1] = 020;
	u.u_base = 0;
	u.u_segflg = 2;
	u.u_procp->p_flag =| SLOCK;
	readi(rp);
	u.u_procp->p_flag =& ~SLOCK;
	u.u_segflg = 0;
	xp->x_flag = XWRIT;
}

/*
 * Assure core for text segment
 * Text must be locked to keep someone else from
 * freeing it in the meantime.
 * x_ccount must be 0.
 */
xexpand(axp)
struct text *axp;
{
	register struct text *xp;

	xp = axp;
	if ((xp->x_caddr = malloc(coremap, xp->x_size)) != NULL) {
		if ((xp->x_flag&XLOAD)==0)
			swap(xp->x_daddr, xp->x_caddr, xp->x_size, B_READ);
		xp->x_ccount++;
		xunlock(xp);
		return;
	}
	savu(u.u_rsav);
	savu(u.u_ssav);
	xswap(u.u_procp, 1, 0);
	xunlock(xp);
	u.u_procp->p_flag =| SSWAP;
	qswtch();
	/* no return */
}

/*
 * Lock and unlock a text segment from swapping
 */
xlock(axp)
{
	register struct text *xp;

	xp = axp;
	while(xp->x_flag&XLOCK) {
		xp->x_flag =| XWANT;
		sleep(xp, PSWP);
	}
	xp->x_flag =| XLOCK;
}

xunlock(axp)
{
	register struct text *xp;

	xp = axp;
	if (xp->x_flag&XWANT)
		wakeup(xp);
	xp->x_flag =& ~(XLOCK|XWANT);
}

/*
 * Decrement the in-core usage count of a shared text segment.
 * When it drops to zero, free the core space.
 */
xccdec(axp)
struct text *axp;
{
	register struct text *xp;

	if ((xp = axp)==NULL || xp->x_ccount==0)
		return;
	xlock(xp);
	if (--xp->x_ccount==0) {
		if (xp->x_flag&XWRIT) {
			xp->x_flag =& ~XWRIT;
			swap(xp->x_daddr,xp->x_caddr,xp->x_size,B_WRITE);
		}
		mfree(coremap, xp->x_size, xp->x_caddr);
	}
	xunlock(xp);
}
