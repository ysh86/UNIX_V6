#
/*
 * VT01 driver via DR11C to 11/20
 */

#include "../param.h"
#include "../user.h"
#include "../userx.h"
#include "../tty.h"

struct
{
	char	iflag;
	char	oflag;
	struct	clist	iq;
} vt;

struct vtreg {
	int	csr;
	int	wbuf;
	int	rbuf;
};

#define	VTADDR	0167770
#define	CSR0	01
#define	CSR1	02
#define	IENABL	0140
#define	SEOF	0100000
#define	VTPRI	8

vtopen(dev, flag)
{
	VTADDR->csr = IENABL;
}

vtclose()
{
	VTADDR->wbuf = SEOF;
	VTADDR->csr =| CSR0;
	while(getc(&vt.iq) >= 0)
		;
}

vtread()
{
	register c;

	spl5();
	do {
		VTADDR->csr =& ~CSR1;
		while((c=getc(&vt.iq)) < 0) {
			vt.iflag++;
			sleep(&vt.iflag, VTPRI);
		}
		if(passc(c) < 0)
			break;
	} while(vt.iq.c_cc > 0);
	spl0();
}

vtwrite()
{
	register int c;
	int register count;

	while ((c=cpass()) >= 0) {
	    retry:
		for (count=0; count<10; count++)
			if ((VTADDR->csr&CSR0)==0) {
				VTADDR->wbuf = c&0377;
				VTADDR->csr =| CSR0;
				goto contin;
			}
		spl5();
		if (VTADDR->csr&CSR0) {
			vt.oflag++;
			sleep(&vt.oflag, VTPRI);
		}
		spl0();
		goto retry;
    contin:;
	}
}

vtaintr()
{
	register c;

	c = VTADDR->rbuf & 0377;
	VTADDR->csr =| CSR1;
	if(vt.iq.c_cc <= 50)
		putc(c, &vt.iq);
	if(vt.iflag) {
		vt.iflag = 0;
		wakeup(&vt.iflag);
	}
	if(vt.iq.c_cc <= 50)
		VTADDR->csr =& ~CSR1;
}

vtbintr()
{
	VTADDR->csr =& ~CSR0;
	if (vt.oflag) {
		vt.oflag = 0;
		wakeup(&vt.oflag);
	}
}
