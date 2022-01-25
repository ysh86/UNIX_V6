#
#include "../param.h"
#include "../systm.h"
#include "../user.h"
#include "../userx.h"
#include "../proc.h"
#include "../procx.h"

#define	UMODE	0170000
#define	SCHMAG	8/10

/*
 * clock is called straight from
 * the real time clock interrupt.
 *
 * Functions:
 *	reprime clock
 *	copy *switches to display
 *	implement callouts
 *	maintain user/system times
 *	maintain date
 *	profile
 *	tout wakeup (sys sleep)
 *	lightning bolt wakeup (every second)
 *	alarm clock signals
 *	jab the scheduler
 */
char	*await { &waitloc };

clock(dev, sp, r1, nps, r0, pc, ps)
{
	register struct callo *p1, *p2;
	register struct proc *pp;
	int a;

	/*
	 * restart clock
	 */

	*lks = 0115;

	/*
	 * display register
	 */

	display();

	/*
	 * callouts
	 * if none, just return
	 * else update first non-zero time
	 */

	if(callout[0].c_func == 0)
		goto out;
	p2 = &callout[0];
	while(p2->c_time<=0 && p2->c_func!=0)
		p2++;
	p2->c_time--;

	/*
	 * if ps is high, just return
	 */

	if((ps&0340) != 0)
		goto out;

	/*
	 * callout
	 */

	spl5();
	if(callout[0].c_time <= 0) {
		p1 = &callout[0];
		while(p1->c_func != 0 && p1->c_time <= 0) {
			(*p1->c_func)(p1->c_arg);
			p1++;
		}
		p2 = &callout[0];
		while(p2->c_func = p1->c_func) {
			p2->c_time = p1->c_time;
			p2->c_arg = p1->c_arg;
			p1++;
			p2++;
		}
	}

	/*
	 * lightning bolt time-out
	 * and time of day
	 */

out:
	a = dk_busy&07;
	if((ps&UMODE) == UMODE) {
		u.u_utime++;
		if(u.u_prof[3])
			incupc(pc, u.u_prof);
		if(u.u_procp->p_nice > 2)
			a =+ 8;
	} else {
		a =+ 16;
		if (pc == await)
			a =+ 8;
		u.u_stime++;
	}
	dk_time[a] =+ 1;
	pp = u.u_procp;
	if(++pp->p_cpu == 0)
		pp->p_cpu--;
	if(++lbolt >= HZ) {
		if((ps&0340) != 0)
			return;
		lbolt =- HZ;
		++time;
		spl1();
		if (time == tout)
			wakeup(&tout);
		runrun++;
			wakeup(&lbolt);
		for(pp = &proc[0]; pp < &proc[NPROC]; pp++)
		if (pp->p_stat && pp->p_stat<SZOMB) {
			if(pp->p_time != 127)
				pp->p_time++;
			if(pp->p_clktim)
				if(--pp->p_clktim == 0)
					psignal(pp, SIGCLK);
			a = (pp->p_cpu & 0377)*SCHMAG + pp->p_nice;
			if(a < 0)
				a = 0;
			if(a > 255)
				a = 255;
			pp->p_cpu = a;
			if(pp->p_pri >= PUSER)
				setpri(pp);
		}
		if(runin!=0) {
			runin = 0;
			wakeup(&runin);
		}
		if((ps&UMODE) == UMODE) {
			u.u_ar0 = &r0;
			if(issig())
				psig();
			pp = u.u_procp;
			if (pp->p_nice==0 && u.u_utime > 60*HZ)
				pp->p_nice = 2;
			setpri(pp);
		}
	}
}

/*
 * timeout is called to arrange that
 * fun(arg) is called in tim/HZ seconds.
 * An entry is sorted into the callout
 * structure. The time in each structure
 * entry is the number of HZ's more
 * than the previous entry.
 * In this way, decrementing the
 * first entry has the effect of
 * updating all entries.
 *
 * The panic is there because there is nothing
 * intelligent to be done if an entry won't fit.
 */
timeout(fun, arg, tim)
{
	register struct callo *p1, *p2;
	register t;
	int s;

	t = tim;
	s = PS->integ;
	p1 = &callout[0];
	spl7();
	while(p1->c_func != 0 && p1->c_time <= t) {
		t =- p1->c_time;
		p1++;
	}
	if (p1 >= &callout[NCALL-1])
		panic("Timeout table overflow");
	p1->c_time =- t;
	p2 = p1;
	while(p2->c_func != 0)
		p2++;
	while(p2 >= p1) {
		(p2+1)->c_time = p2->c_time;
		(p2+1)->c_func = p2->c_func;
		(p2+1)->c_arg = p2->c_arg;
		p2--;
	}
	p1->c_time = t;
	p1->c_func = fun;
	p1->c_arg = arg;
	PS->integ = s;
}
